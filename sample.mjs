const u16 = new Uint16Array([0, 0xffff]);
const buf = Buffer.copyBytesFrom(u16, 1, 1);
u16[1] = 0;
console.log(buf);
console.log(buf.length); // 2
console.log(buf[0]); // 255
console.log(buf[1]); // 255 
