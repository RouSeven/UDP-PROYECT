#!/bin/bash
echo "Instalando dependencias..."
sudo apt update
sudo apt install -y build-essential nodejs npm

echo "Instalando módulos de Node..."
npm install

echo "Compilando servidor UDP..."
cd udp_server && make && cd ..

echo "Listo. Ejecuta el servidor con:"
echo "  node bridge.js"
echo "  ./udp_server/server"
