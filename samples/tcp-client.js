import { Socket } from 'std:net';

const socket = new Socket();
socket.ondata = (chunk) => {
  console.log(chunk.toString('utf8'));
  socket.write(chunk);
}

socket.onerror = (err) => {
  console.log("Got error:", err);
}

socket.onclose = () => {
  console.log("connection closed?", socket.destroyed);
}

setTimeout(() => {
  socket.destroy();
  console.log("connection closed?", socket.destroyed);
}, 6000);

await socket.connect('127.0.0.1', 8000);
