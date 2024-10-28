import { Server } from 'std:net';
import { uuidv4 } from 'std:uuid';

const server = new Server();

server.onconnection = function(socket) {
  socket.ondata = (chunk) => {
    const res = uuidv4();
    socket.write(`HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: ${res.length}\r\n\r\n${res}`);
    socket.shutdown();
  };

  socket.onerror = (err) => {
    console.log('unexpected error');
    console.log(err);
  }

  socket.onclose = () => {
    //console.log("connection closed");
  };
}

server.listen(5000);
