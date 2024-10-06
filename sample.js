import { assert, assertStrictEqual, assertThrows } from 'std:assert';
import { tcpListen } from 'std:net';
import path from 'std:path';

console.log(`Basename: ${path.basename(import.meta.filename)}`);
console.log(`Resolved: ${import.meta.resolve('./in%ex.js')}`);

function willThrow() {
  throw new Error("Some error");
}

Runtime.argv.forEach(item => {
  console.log(`Arg: ${item}`);
});

assertThrows(willThrow);
assert(2 + 2 == 4);
assertStrictEqual(2, 2);

console.log(`Platform is ${Runtime.platform} and pid is: ${Runtime.pid}`);
console.log(`Cwd is: ${Runtime.cwd()} and runtime version is: ${Runtime.version}`);
console.log("Hi");
console.log(import.meta.resolve('./'));
console.log(`Dirname: ${import.meta.dirname}`)
console.log(`Filename: ${import.meta.filename}`);
console.log(`Url: ${import.meta.url}`);

tcpListen((socket) => {
  console.log("Client connected");

  socket.read((chunk) => {
    console.log(`Received data`);
    const response = `<h1>${chunk}</h1>`; // echo HTTP request
    socket.write(`HTTP/1.1 200 OK\r\nContent-Length: ${response.length}\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n`);
    socket.write(response);
    socket.close();
    socket.setTimeout(60 * 1000);
  });
}, 8000);
