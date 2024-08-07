const net = require('net');

const server = net.createServer((socket) => {
  console.log("Client connected");

  socket.on('data', (chunk) => {
    console.log(`Received data`);
    const response = `<h1>${chunk}</h1>`; // echo HTTP request
    socket.write(`HTTP/1.1 200 OK\r\nContent-Length: ${response.length}\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n`);
    socket.write(response);
    socket.destroy();
  })
});

server.listen(8000);
