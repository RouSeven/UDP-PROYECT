// bridge.js
const express = require('express');
const WebSocket = require('ws');
const dgram = require('dgram');
const path = require('path');

const app = express();
const wss = new WebSocket.Server({ noServer: true });
const udpClient = dgram.createSocket('udp4');

const UDP_PORT = 4950;
const UDP_HOST = '192.168.1.2'; // IP del servidor en C
  
// Servir los archivos HTML/JS/CSS
app.use(express.static(path.join(__dirname, 'public')));

const server = app.listen(3000, () => {
  console.log('Servidor web corriendo en http://localhost:3000');
});

// Vincular WebSocket
server.on('upgrade', (req, socket, head) => {
  wss.handleUpgrade(req, socket, head, ws => {
    wss.emit('connection', ws, req);
  });
});

wss.on('connection', ws => {
  ws.on('message', msg => {
    console.log(`Mensaje recibido desde cliente web: ${msg}`);

    const messageBuffer = Buffer.from(msg);

    udpClient.send(messageBuffer, UDP_PORT, UDP_HOST, err => {
      if (err) console.error(err);
    });

    udpClient.once('message', response => {
      ws.send(response.toString());
    });
  });
});
