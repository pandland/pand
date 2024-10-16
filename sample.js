const { TcpStream, TcpServer } = Runtime.bind("tcp");

const server = new TcpServer();
server.listen(5000);
