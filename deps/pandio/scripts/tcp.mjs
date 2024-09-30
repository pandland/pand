import { createServer } from 'net';

createServer((socket) => {
  console.log('Client connected');

  socket.on('data', (data) => {
    console.log(data.toString());
    socket.write("HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello");
  });

  socket.on('end', () => {
    socket.destroy();
  })
}).listen(8000);
