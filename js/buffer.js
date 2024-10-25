const { fillRandom, fromString, decode } = Runtime.bind("buffer");

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
    if (buffers.length === 0)
      return new Buffer(0);

    const size = buffers.reduce((total, item) => total + item.length, 0);
    const final = new Buffer(size);
    let offset = 0;

    buffers.forEach(item => {
      if (item instanceof Uint8Array) {
        final.set(item, offset);
        offset += item.length;
      } else {
        throw new Error("Expected Uint8Array instance");
      }
    });

    return final;
  }

  toString() {
    return decode(this.buffer);
  }
}
