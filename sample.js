const { TcpStream, TcpServer } = Runtime.bind("tcp");

const server = new TcpServer();
server.onConnection = (stream) => {
  console.log("New connection.\n");
  stream.onData = (data) => {
    stream.write(data);
    stream.shutdown();
  };
  stream.onClose = () => {
    console.log("Connection closed")
  };
}

server.listen(5000);
