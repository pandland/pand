const { TcpServer, TcpStream } = Runtime.bind("tcp");


const client = new TcpStream();
client.onConnect = () => {
  client.write("Hello, server\n");

  client.onData = (data) => {
    console.log(`From server: ${data}`);
    client.shutdown();
  }

  client.onClose = () => {
    console.log("connection closed :o");
  }
}

client.connect("127.0.0.1", 8000);

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
