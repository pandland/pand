> ⚠️ Early stage of development.

# done.js

My own JavaScript runtime - currently, just randomly messing around with v8 engine in C++. Very unstable.

Once I will figure out how things work together - I will rewrite this project.

### TODO:

- [x] Try to bind SWC (written in Rust) into my C++ codebase
- [x] Integrate my async IO library for runtime's event loop: [luxio](https://github.com/michaldziuba03/luxio).
- [x] Create timers
- [x] Clunky TCP support
- [x] Basic support for ES6 imports

> Current state example:

```js
import { 
  assert, 
  assertStrictEqual, 
  assertThrows 
} from 'std:assert';
import { tcpListen } from 'std:net';

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
  });
}, 8000);

```

