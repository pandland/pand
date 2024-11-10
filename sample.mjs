import { open, O_RDONLY } from 'std:fs';

const file = await open("../test.md", O_RDONLY);
const buf = new Buffer(1024);
const size = await file.read(buf);
console.log('size:', size);
console.log(buf);
await file.close();
