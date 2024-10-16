const { TcpStream } = Runtime.bind("tcp");

const stream = new TcpStream();
stream.connect("127.0.0.1", 8000);
stream.pause();

setTimeout(() => {
  stream.resume();
}, 10 * 1000);
