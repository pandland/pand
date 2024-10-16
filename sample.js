const { TcpStream } = Runtime.bind("tcp");

const stream = new TcpStream();
stream.connect("127.0.0.1", 8000);
stream.pause();

stream.onData = (data) => {
  console.log(`Received data:\n${data}`);
}

setTimeout(() => {
  stream.write("Hello from JavaScript land\n");
  stream.resume();

  setTimeout(() => {
    stream.shutdown();
  }, 1000);
}, 3000);
