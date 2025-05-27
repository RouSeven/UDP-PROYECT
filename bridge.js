const express = require('express');
const WebSocket = require('ws');
const dgram = require('dgram');
const path = require('path');
const fs = require('fs');

const app = express();
const wss = new WebSocket.Server({ noServer: true });
const udpClient = dgram.createSocket('udp4');

const UDP_PORT = 4950;
const UDP_HOST = '192.168.1.2'; // Cambia esta IP a la de tu servidor C

const uploadsDir = path.join(__dirname, 'public', 'uploads');
if (!fs.existsSync(uploadsDir)) {
  fs.mkdirSync(uploadsDir, { recursive: true });
}

// Servir archivos estáticos
app.use(express.static(path.join(__dirname, 'public')));

// Servir archivos en /uploads (mostrar imagenes, forzar descarga en otros)
app.get('/uploads/:filename', (req, res) => {
  const filePath = path.join(uploadsDir, req.params.filename);
  const ext = path.extname(filePath).toLowerCase();
  const imageTypes = ['.jpg', '.jpeg', '.png', '.gif', '.webp'];

  if (imageTypes.includes(ext)) {
    res.sendFile(filePath); // mostrar en navegador
  } else {
    res.download(filePath); // forzar descarga
  }
});

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
        const decoded = Buffer.from(parsed.content, 'base64');
        const safeName = Date.now() + '-' + parsed.filename;
        const filePath = path.join(uploadsDir, safeName);
        fs.writeFileSync(filePath, decoded);

        const notifyMessage = `[${parsed.username}] envió archivo: ${parsed.filename}`;
        udpClient.send(notifyMessage, UDP_PORT, UDP_HOST, err => {
          if (err) console.error('Error al notificar al servidor C:', err);
        });

        const ext = path.extname(parsed.filename).toLowerCase();
        const publicURL = `/uploads/${safeName}`;
        const imageTypes = ['.jpg', '.jpeg', '.png', '.gif', '.webp'];

        let htmlMessage = "";
        if (imageTypes.includes(ext)) {
          htmlMessage = `[${parsed.username}] envió una imagen:<br>
            <img src="${publicURL}" style="max-width:200px;" /><br>
            <a href="${publicURL}" download>Descargar imagen</a>`;
        } else {
          htmlMessage = `[${parsed.username}] envió archivo: <a href="${publicURL}" download>${parsed.filename}</a>`;
        }

        for (let client of clients) {
          if (client.readyState === WebSocket.OPEN) {
            client.send(htmlMessage);
          }
        }

      } else {
        // Mensaje de texto simple
        const buffer = Buffer.from(msg);
        udpClient.send(buffer, UDP_PORT, UDP_HOST, err => {
          if (err) console.error('Error al enviar al servidor C:', err);
        });
      }

    } catch (err) {
      // Si no es JSON, tratarlo como texto
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
  for (let client of clients) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(msg);
    }
  }
});
