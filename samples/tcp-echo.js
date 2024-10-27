import { Server } from 'std:net';

const server = new Server();

server.onconnection = function(socket) {
  socket.ondata = (chunk) => {
    console.log(chunk.toString('utf8'));
    socket.write(chunk);
    socket.shutdown();
  };

  socket.onclose = () => {
    console.log("connection closed");
  };
}

server.listen(5000);
console.log(`Starting server on port: ${server.port}`);
