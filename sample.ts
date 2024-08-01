tcpListen((socket) => {
  socket.write("HTTP/1.1 200 OK\r\nContent-Length: 22\r\nContent-Type: text/html\r\n\r\n<h1>Hello world!</h1>");
  println("Client connected");

  setTimeout(socket.close, 10 * 1000);
}, 8000);
