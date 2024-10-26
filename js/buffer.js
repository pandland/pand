/* Copyright (c) Michał Dziuba
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

"use strict";

const { fillRandom, memcmp } = Runtime.bind("buffer");
const transcoder = Runtime.bind("transcoder");

const encoders = {
  utf8: transcoder.UTF8,
  "utf-8": transcoder.UTF8,
  ascii: transcoder.ASCII,
  base64: transcoder.BASE64,
  base64url: transcoder.BASE64URL,
  latin: transcoder.LATIN,
  hex: transcoder.HEX,
};

export class Buffer extends Uint8Array {
  static random(size) {
    const buffer = new Buffer(size);
    fillRandom(buffer.buffer);

    return buffer;
  }

  static from(...args) {
    const value = args[0];
    if (value && typeof value === "string") {
      const option = encoders[args[1]] || encoders.utf8;
      const inner = transcoder.encode(value, option);
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
        throw new Error("Expected <Uint8Array> instance");
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

  static isEncoding(encoding) {
    if (encoding && encoders[encoding]) return true;

    return false;
  }

  static compare(a, b) {
    if (!(a instanceof Uint8Array) || !(b instanceof Uint8Array)) {
      throw new Error("Buffers must be <Buffer> or <Uint8Array>");
    }

    if (a === b) return 0;

    const result = memcmp(a.buffer, b.buffer);
    if (result === 0) {
      if (a.byteLength > b.byteLength) return 1;
      if (a.byteLength < b.byteLength) return -1;
    } else {
      return result > 0 ? 1 : -1;
    }

    return result;
  }

  equals(other) {
    if (!(other instanceof Uint8Array)) {
      throw new Error("Other buffer must be a <Buffer> or <Uint8Array>");
    }
    // check only references (this is the same object)
    if (this === other) return true;

    if (this.length !== other.length) return false;

    return memcmp(this.buffer, other.buffer) === 0;
  }

  toString(encoding) {
    const option = encoders[encoding] || encoders.utf8;
    return transcoder.decode(this.buffer, option);
  }

  toJSON() {
    if (this.length == 0) {
      return { type: "Buffer", data: [] };
    }

    const data = Array.from(this);
    return { type: "Buffer", data };
  }
}
