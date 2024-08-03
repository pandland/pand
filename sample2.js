const hello = require('./hello.js')

exports.saySample2 = () => {
  hello.sayHello('sample2');
  println("From sample2");
}
