<p align="center">
  <a href="https://pandland.github.io"><img src="https://github.com/user-attachments/assets/40c000fa-26b2-425d-98d0-ad68d3026b0e" alt="Logo" height=170></a>
</p>

<h1 align="center">PandJS</h1>

<div align="center">
  <a href="https://github.com/pandland/pand/tree/main/docs">Documentation</a>
  <span>&nbsp;&nbsp;•&nbsp;&nbsp;</span>
  <a href="https://pandland.github.io/pand">Website</a>
  <span>&nbsp;&nbsp;•&nbsp;&nbsp;</span>
  <a href="https://github.com/pandland/pand/issues/new">Issues</a>
  <br />
  <br />
</div>

> ⚠️ Early stage of development.

My own JavaScript runtime - currently, just randomly messing around with v8 engine in C++. Very unstable as I learn how things work together.

### Clone

```sh
git clone https://github.com/pandland/pand.git
```

### Building

Make sure you have CMake installed. Build infrastructure is still in early stage, currently we rely on newest version of v8 - this is not what I want, but for now it's fine.

You need to download v8 as dependency and it will take reasonable amount of time to compile (even 40 mins - depends on machine).

Windows build scripts are still in progress (and this is terrible experience).

```sh
# Download v8 and build
cmake -P v8.cmake

# Build our actual project
mkdir build
cd build

cmake ..
cmake --build .
```

> `v8.cmake` script is not tested yet on fresh Linux installation.

### Code sample

##### Currently working on providing helpful messages for working with code, like this:

![image](https://github.com/user-attachments/assets/14d06de3-8605-4f2c-b063-a0259cdb674d)


> Current state example:

```js
import { 
  assert, 
  assertThrows,
  assertStrictEqual  
} from 'std:assert';
import { tcpListen } from 'std:net';

function willThrow() {
  throw new Error("Some error");
}

Runtime.argv.forEach(item => {
  console.log(`Arg: ${item}`);
});

assert(2 + 2 == 4);
assertThrows(willThrow);
assertStrictEqual(2, 2);

console.log(`Platform is ${Runtime.platform} and pid is: ${Runtime.pid}`);
console.log(`Cwd is: ${Runtime.cwd()} and runtime version is: ${Runtime.version}`);
console.log(`Dirname: ${import.meta.dirname}`)
console.log(`Filename: ${import.meta.filename}`);
console.log(`Url: ${import.meta.url}`);

tcpListen((socket) => {
  console.log("Client connected");

  setTimeout(() => {
    socket.close();
  }, 5 * 1000);

  socket.read((chunk) => {
    console.log(`Received data`);
    const response = `<h1>${chunk}</h1>`; // echo HTTP request
    socket.write(`HTTP/1.1 200 OK\r\nContent-Length: ${response.length}\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n`);
    socket.write(response);
  });
}, 8000);
```

## License

Distributed under the MIT License. See `LICENSE` for more information.
