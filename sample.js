const { TcpStream } = Runtime.bind("tcp");

const stream = new TcpStream();
stream.connect("127.0.0.1", 8000);

setTimeout(() => {
  stream.shutdown();
}, 3000);
