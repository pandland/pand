const { TcpStream } = Runtime.bind("tcp");

const stream = new TcpStream();
stream.connect("127.0.0.1", 8000);
stream.pause();

setTimeout(() => {
  stream.write("Hello from JavaScript land\n");
  stream.resume();

  setTimeout(() => {
    stream.write("Goodbye\n");
    stream.shutdown();
  }, 1000);
}, 3000);
