interface Socket {
  write: (data: string) => void;
  read: (cb: (chunk: string) => void) => void;
  close: () => void;
}

tcpListen((socket: Socket) => {
  println("Client connected");

  socket.read((chuk) => {
    println(`Received data: ${chuk}`);
    socket.write("HTTP/1.1 200 OK\r\nContent-Length: 21\r\nContent-Type: text/html\r\nConnection: close\r\n");
    socket.write("\r\n<h1>Hello world!</h1>");
    socket.close();
  });
}, 8000);

setInterval(() => {
  println("Repeat");
}, 2000);
