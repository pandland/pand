tcpListen((socket) => {
  println("Client connected");

  socket.read(() => {
    println("Received data");
    socket.write("HTTP/1.1 200 OK\r\nContent-Length: 21\r\nContent-Type: text/html\r\n");
    socket.write("\r\n<h1>Hello world!</h1>");
  });
}, 8000);
