const { sayHello } = require("./hello.js");
const { saySample2 } = require('./sample2.js');

sayHello("From another module!!!!");
saySample2();
println(`__dirname is: ${__dirname} and __filename is: ${__filename}`);

function getPathFromRequest(chunk) {
  const line = chunk.split('\r\n')[0];
  if (!line) {
    return "Invalid";
  }

  const path = line.split(' ')[1];
  if (!path) {
    return "Invalid";
  }

  return path;
}

const net = bind("tcp");

net.tcpListen((socket) => {
  println("Client connected");

  socket.read((chunk) => {
    const path = getPathFromRequest(chunk);
    println(`Received data`);
    const response = `<h1>Path: ${path}</h1>`;
    socket.write(`HTTP/1.1 200 OK\r\nContent-Length: ${response.length}\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n`);
    socket.write(response);
    socket.close();
  });
}, 8000);
