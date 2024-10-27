const { TcpStream } = Runtime.bind('tcp');

const client = new TcpStream();

client.onData = (chunk) => {
  console.log(new Buffer(chunk).toString());
  client.write(Buffer.from('Michał').buffer);
} 

client.onConnect = () => {
  console.log("Connected");
}

client.onClose = () => {
  console.log('Connection closed');
}

client.onError = (err) => {
  throw err;
}

client.connect('127.0.0.1', 8000);
