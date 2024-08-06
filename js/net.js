const net = Runtime.bind("tcp");

class Socket {
  constructor(handle) {
    this._handle = handle;
    this.closed = false;
  }

  read(cb) {
    this._handle.onread = cb;
  }

  close() {
    if (this._handle) {
      this._handle.close();
      this.closed = true;
    }
    // otherwise - ignore and make it safe to call multiple times
  }

  write(data) {
    if (this._handle) {
      this._handle.write(data);
    }
  }
}

export const tcpListen = (callback, port) => {
  net.tcpListen((handle) => {
    const socket = new Socket(handle);
    handle.onclose = () => {
      socket._handle = null;
    }
    handle.onread = (chunk) => {} // user should call socket.read() to overwrite this callback
    
    callback(socket);
  }, port);
};
