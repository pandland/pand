const { TcpServer, TcpStream } = Runtime.bind("tcp");

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

setTimeout((arg1) => {
  console.log(`Timeout with arg: ${arg1}`);
  server.close();
}, 5000, 2137);
