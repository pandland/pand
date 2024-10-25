const { fillRandom, fromString, decode, memcmp } = Runtime.bind("buffer");

export class Buffer extends Uint8Array {
  static random(size) {
    const buffer = new Buffer(size);
    fillRandom(buffer.buffer);

    return buffer;
  }

  static from(...args) {
    const value = args[0];
    if (value && typeof value === "string") {
      const inner = fromString(value);
      return new Buffer(inner);
    }

    return super.from(...args);
  }

  /**
   * Creates one buffer from multiple buffers (concatenates them).
   * It's diffent from Buffer.concat from Node.js
   *
   * @param {Buffer[]} buffers - buffers to concat
   * @throws {Error}
   */
  static concat(...buffers) {
    if (buffers.length === 0) return new Buffer(0);

    const size = buffers.reduce((total, buffer) => total + buffer.length, 0);
    const final = new Buffer(size);
    let offset = 0;

    for (const buffer of buffers) {
      if (buffer instanceof Uint8Array) {
        final.set(buffer, offset);
        offset += buffer.length;
      } else {
        throw new Error("Expected Uint8Array instance");
      }
    }

    return final;
  }

  static copyBytesFrom(view, offset, length) {
    if (!ArrayBuffer.isView(view)) {
      throw new TypeError("View must be a <TypedArray> instance");
    }

    let end;
    if (length === undefined) {
      end = view.length;
      length = view.length;
    }

    if (offset === undefined || offset < 0) {
      offset = 0;
    }

    if (offset >= view.length || length < 0) {
      return new Buffer(0);
    }

    if (!end) {
      end = offset + length;
    }

    const slice = view.slice(offset, end);
    return Buffer.from(
      new Uint8Array(slice.buffer, slice.byteOffset, slice.byteLength)
    );
  }

  static isBuffer(obj) {
    return obj instanceof Buffer;
  }

  equals(other) {
    if (!(other instanceof Uint8Array)) {
      throw new Error("Other buffer must be a Buffer or Uint8Array");
    }
    // check only references (this is the same object)
    if (this === other) return true;

    if (this.length !== other.length) return false;

    return memcmp(this.buffer, other.buffer) === 0;
  }

  toString() {
    return decode(this.buffer);
  }

  toJSON() {
    if (this.length == 0) {
      return { type: "Buffer", data: [] };
    }

    const data = Array.from(this);
    return { type: "Buffer", data };
  }
}
