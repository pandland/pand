const fs = Runtime.bind("fs");

export const O_CREAT = fs.O_CREAT;
export const O_RDONLY = fs.O_RDONLY
export const O_WRONLY = fs.O_WRONLY;
export const O_APPEND = fs.O_APPEND;
export const O_EXCL = fs.O_EXCL;
export const O_RDWR = fs.O_RDWR;
export const O_TRUNC = fs.O_TRUNC;

export async function open(path, flags, mode) {
    if (typeof path !== 'string') {
        throw new TypeError('Path must be a string');
    }

    if (flags && !Number.isInteger(flags)) {
        throw new TypeError("Flags must be an integer");
    }

    if (mode && !Number.isInteger(mode)) {
        throw new TypeError("Mode must be an integer");
    }

    const handle = new fs.File();
    const fd = await handle.open(path, flags, mode);

    return new FileHandle(handle, fd);
}

export class FileHandle {
    #handle = null;

    constructor(handle, fd) {
        this.#handle = handle;
        this.fd = fd;
    }

    read(buffer) {
        return this.#handle.read(buffer);
    }

    write(buffer) {
        return this.#handle.write(buffer);
    }

    close() {
        return this.#handle.close();
    }
}
