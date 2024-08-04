const { saySample2 } = require('./sample2.js')

println(`hello.js: __dirname is: ${__dirname} and __filename is: ${__filename}`);

exports.sayHello = (name) => {
  println(`Hello ${name}`);
}
