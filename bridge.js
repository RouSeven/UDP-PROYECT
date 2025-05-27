const express = require('express');
const WebSocket = require('ws');
const dgram = require('dgram');
const path = require('path');
const fs = require('fs');

const app = express();
const wss = new WebSocket.Server({ noServer: true });
const udpClient = dgram.createSocket('udp4');

const UDP_PORT = 4950;
const UDP_HOST = '192.168.1.2'; // Reemplaza con la IP de tu servidor C

// Crear ca rpeta "uploads" si no existe
const uploadsDir = path.join(__dirname, 'public', 'uploads');
if (!fs.existsSync(uploadsDir)) {
  fs.mkdirSync(uploadsDir, { recursive: true });
}

// Servir archivos estáticos
app.use(express.static(path.join(__dirname, 'public')));

const server = app.listen(3000, () => {
  console.log('Servidor web corriendo en http://localhost:3000');
});

server.on('upgrade', (req, socket, head) => {
  wss.handleUpgrade(req, socket, head, ws => {
    wss.emit('connection', ws, req);
  });
});

const clients = new Set();

wss.on('connection', ws => {
  clients.add(ws);
  console.log('Nuevo cliente WebSocket conectado.');

  ws.on('message', msg => {
    try {
      const parsed = JSON.parse(msg);

      if (parsed.type === 'file') {
        console.log(`Archivo recibido: ${parsed.filename} de ${parsed.username}`);

        const decoded = Buffer.from(parsed.content, 'base64');
        const safeName = Date.now() + '-' + parsed.filename;
        const filePath = path.join(uploadsDir, safeName);

        // Guardar el archivo
        fs.writeFileSync(filePath, decoded);

        // Notificar al servidor UDP (solo mensaje, no archivo)
        const notifyMessage = `[${parsed.username}] envió archivo: ${parsed.filename}`;
        udpClient.send(notifyMessage, UDP_PORT, UDP_HOST, err => {
          if (err) console.error('Error al notificar al servidor C:', err);
        });

        // Notificar al cliente WebSocket con un enlace al archivo
        const publicURL = `/uploads/${safeName}`;
        const htmlLink = `[${parsed.username}] envió archivo: <a href="${publicURL}" target="_blank">${parsed.filename}</a>`;
        for (let client of clients) {
          if (client.readyState === WebSocket.OPEN) {
            client.send(htmlLink);
          }
        }

      } else {
        // Texto normal
        const buffer = Buffer.from(msg);
        udpClient.send(buffer, UDP_PORT, UDP_HOST, err => {
          if (err) console.error('Error al enviar al servidor C:', err);
        });
      }

    } catch (err) {
      // No es JSON → texto plano
      const buffer = Buffer.from(msg);
      udpClient.send(buffer, UDP_PORT, UDP_HOST, err => {
        if (err) console.error('Error al enviar al servidor C:', err);
      });
    }
  });

  ws.on('close', () => {
    clients.delete(ws);
    console.log('Cliente desconectado.');
  });
});

udpClient.on('message', response => {
  const msg = response.toString();
  console.log(`Respuesta del servidor C: ${msg}`);

  for (let client of clients) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(msg);
    }
  }
});
