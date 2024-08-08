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

  pause() {
    if (this._handle) {
      this._handle.pause();
    }
  }

  resume() {
    if (this._handle) {
      this._handle.resume();
    }
  }

  setTimeout(delay) {
    if (!Number.isInteger(delay)) {
      throw new Error('Invalid socket timeout.');
    }

    if (this._handle) {
      this._handle.setTimeout(delay);
      return true;
    }

    return false;
  }

  write(data) {
    if (this._handle) {
      this._handle.write(data);
    }
  }
}

function toPort(value) {
  const port = Number(value);
  
  if (!Number.isInteger(port)) {
      throw new Error('Port must be an integer.');
  }

  if (port < 0 || port > 65535) {
      throw new Error('Port must be in the range 0-65535.');
  }
  
  return port;
}

class Server {
  constructor() {
    this.connections = 0;
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
  }, toPort(port));
};
