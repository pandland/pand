const value = Buffer.from("TWFudWFsbHlUZXN0ZWQvUHJvcGVybHk", 'base64url');
console.log(value.toString('utf8'));
console.log(value.toString('hex'));
console.log(value.toString('base64'));
console.log(value.toString('base64url'));
