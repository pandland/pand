const hello = require('./hello.js')

exports.saySample2 = () => {
  println("From sample2");
  hello.sayHello("sample2");
}
