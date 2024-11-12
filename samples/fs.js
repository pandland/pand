import { open, O_RDONLY } from 'std:fs';

const file = await open("./README.md", O_RDONLY);
const buf = new Buffer(1024);
const size = await file.read(buf);
console.log('size:', size);
const content = buf.subarray(0, size)
console.log(content);
console.log(content.toString('utf8'))

await file.close();
