const { TcpServer, TcpStream } = Runtime.bind("tcp");

const client = new TcpStream();

client.onError = (err) => {
  console.log(err.code);
  console.log(err.message);
  // issue: without destroy() we will get onConnect, because of bug in pandio
  client.destroy();
}

client.onConnect = () => {
  console.log("Connected");
}

client.onData = (data) => {
  console.log(data);
}

client.onClose = () => {
  console.log("Close")
}

client.connect("127.0.0.1", 4000);
