const express = require('express');
const WebSocket = require('ws');
const dgram = require('dgram');
const path = require('path');

const app = express();
const wss = new WebSocket.Server({ noServer: true });
const udpClient = dgram.createSocket('udp4');

const UDP_PORT = 4950;
const UDP_HOST = '192.168.1.2'; // IP del servidor en C

// Servir archivos estáticos (HTML/JS/CSS)
app.use(express.static(path.join(__dirname, 'public')));

const server = app.listen(3000, () => {
  console.log('Servidor web corriendo en http://localhost:3000');
});

// Manejar upgrade a WebSocket
server.on('upgrade', (req, socket, head) => {
  wss.handleUpgrade(req, socket, head, ws => {
    wss.emit('connection', ws, req);
  });
});

// Lista de todos los clientes WebSocket conectados
const clients = new Set();

wss.on('connection', ws => {
  clients.add(ws);
  console.log('Nuevo cliente WebSocket conectado.');

  ws.on('message', msg => {
    console.log(`Mensaje recibido desde cliente web: ${msg}`);

    const messageBuffer = Buffer.from(msg);

    // Enviar mensaje al servidor en C
    udpClient.send(messageBuffer, UDP_PORT, UDP_HOST, err => {
      if (err) console.error('Error enviando a servidor C:', err);
    });
  });

  ws.on('close', () => {
    clients.delete(ws);
    console.log('Cliente desconectado.');
  });
});

// Cuando el servidor en C responde, reenviar a todos los clientes WebSocket
udpClient.on('message', response => {
  const responseText = response.toString();
  console.log(`Respuesta recibida del servidor C: ${responseText}`);

  for (let client of clients) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(responseText);
    }
  }
});
