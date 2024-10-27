const { TcpStream, TcpServer } = Runtime.bind('tcp');

const kEmpty = 0;
const kConnecting = 1;
const kActive = 2;
const kDestroyed = 3;
const kListening = 4;

export class Socket {
  #handle = null;
  #queue = [];
  #state = kEmpty;

  onerror = null;
  ondata = null;
  onclose = null;

  __init(handle) {
    this.#handle = handle;
    this.#state = kActive;
    this.#handle.onError = this.#onError.bind(this);
    this.#handle.onData = this.#onData.bind(this);
    this.#handle.onClose = this.#onClose.bind(this);
  }

  get connecting() {
    return this.#state === kConnecting;
  }

  get destroyed() {
    return this.#state === kDestroyed;
  }

  // default handler
  #onError(err) {
    if (this.#queue.length > 0) {
      const promise = this.#queue.shift();
      promise.reject(err);
    }

    if (this.onerror) {
      this.onerror(err);
    } else {
      throw err;
    }
  }

  #onClose() {
    if (this.#queue.length > 0) {
      const promise = this.#queue.shift();
      promise.reject(new Error('Connection closed by peer'));
    }
    this.#handle = null;
    this.#state = kDestroyed;

    if (this.onclose) {
      this.onclose();
    }
  }

  #onData(chunk, size) {
    const buf = new Buffer(chunk, 0, size);
    if (this.#queue.length > 0) {
      const promise = this.#queue.shift();
      promise.resolve(buf);
    }

    if (this.ondata) {
      this.ondata(buf);
    }
  }

  pause() {
    if (this.#handle) {
      this.#handle.pause();
    }
  }

  resume() {
    if (this.#handle) {
      this.#handle.resume();
    }
  }

  destroy() {
    if (this.#handle) {
      this.#handle.destroy();
    }
  }

  shutdown() {
    if (this.#handle) {
      this.#handle.shutdown();
    }
  }

  read() {
    if (!this.#handle) {
      throw new Error("Socket is not active");
    }

    return new Promise((resolve, reject) => {
      this.#queue.push({ resolve, reject });
    });
  }

  write(chunk, encoding) {
    if (!this.#handle) {
      throw new Error("Socket is not active");
    }

    if (!chunk) {
      throw new TypeError("Write buffer is required");
    }

    let buf;
    if (typeof chunk === 'string') {
      buf = Buffer.from(chunk, encoding);
    }
    else if (chunk instanceof Uint8Array) {
      buf = chunk;
    }

    if (!buf) {
      throw new TypeError('Write buffer must be a string, <Uint8Array> or <Buffer>');
    }

    this.#handle.write(buf);
  }

  connect(dest, port) {
    if (this.destroyed) {
      throw new Error("Socket is destroyed");
    }

    if (this.#handle) {
      throw new Error("Socket is already initialized");
    }

    this.#state = kConnecting;
    const handle = new TcpStream();
    this.#handle = handle;
    handle.onClose = () => {};
    handle.connect(dest, port);
    return new Promise((resolve, reject) => {
      handle.onConnect = () => {
        this.__init(handle);
        this.#handle.onConnect = null;
        resolve();
      };

      handle.onError = (err) => reject(err);
    });
  }

  setNoDelay(enable) {
    if (this.#handle) {
      this.#handle.setNoDelay(enable);
    }
  }

  setKeepAlive(enable, delay) {
    if (this.#handle) {
      this.#handle.setKeepAlive(enable, delay);
    }
  }
}

export class Server {
  #state = kEmpty;
  #handle = null;

  port = null;
  onconnection = null;

  get listening() {
    return this.#state === kListening;
  }

  #handleConnection(handle) {
    const socket = new Socket();
    socket.__init(handle);

    if (this.onconnection) {
      this.onconnection(socket);
    } else {
      socket.destroy();
    }
  }

  listen(port) {
    if (!port || !Number.isInteger(port)) {
      throw new Error("Port must be an integer");
    }

    if (this.#handle) {
      throw new Error("This server is already listening");
    }

    const handle = new TcpServer();
    handle.onConnection = this.#handleConnection.bind(this);
    handle.listen(port);
    this.#state = kListening;
    this.port = port;
  }

  close() {
    this.#handle.close();
    this.#handle = null;
    this.#state = kDestroyed;
  }
}
