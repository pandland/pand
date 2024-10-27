const value = "Michał";
const buf = Buffer.from(value, 'utf8');

console.log(buf);
console.log(buf.toString('utf8'));
console.log(buf.toString('hex'));
console.log(buf.toString('base64'));
console.log(buf.toString('base64url'));
