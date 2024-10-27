import { Socket } from 'std:net';

const socket = new Socket();
await socket.connect('127.0.0.1', 8000);

socket.onclose = () => {
  console.log(`Connection is destroyed? ${socket.destroyed}`);
}

socket.write("Hello World\n");
socket.write(Buffer.from([63, 64, 64, 63]));

socket.ondata = (chunk) => {
  console.log(chunk);
}

const message = await socket.read();
console.log(message.toString());

console.log(socket.destroyed);
socket.shutdown();
