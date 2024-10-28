import { Server } from 'std:net';

const server = new Server();

server.onconnection = function(socket) {
  socket.ondata = (chunk) => {
    console.log(chunk.length, chunk.buffer.byteLength);
    socket.write(chunk);
    socket.shutdown();
  };

  socket.onclose = () => {
    console.log("connection closed");
  };
}

server.listen(5000);
