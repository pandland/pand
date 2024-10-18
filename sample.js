const { TcpServer, TcpStream } = Runtime.bind("tcp");

const client = new TcpStream();

async function willThrow() {
  throw new Error("async error");
}

willThrow().catch((err) => {
  console.log(`Catch: ${err.message}`);
})

client.onError = (err) => {
  console.log(err.code);
  console.log(err.message);
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
