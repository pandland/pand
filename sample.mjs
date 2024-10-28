import { Server } from 'std:net';

const server = new Server();

server.onconnection = function(socket) {
  socket.ondata = (chunk) => {
    socket.write("HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 5\r\n\r\nHello");
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
