const { fillRandom, fromString, decode } = Runtime.bind("buffer");

export class Buffer extends Uint8Array {
  static random(size) {
    const buffer = new Buffer(size);
    fillRandom(buffer.buffer);

    return buffer;
  }

  static from(...args) {
    const value = args[0];
    if (value && typeof value === 'string') {
      const inner = fromString(value);
      return new Buffer(inner);
    }

    return super.from(...args);
  }

  toString() {
    return decode(this.buffer);
  }
}
