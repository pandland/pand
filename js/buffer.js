const { fillRandom } = Runtime.bind("buffer");

export class Buffer extends Uint8Array {
  static random(size) {
    const buffer = new Buffer(size);
    fillRandom(buffer.buffer);  // function expects ArrayBuffer instance

    return buffer;
  }
}
