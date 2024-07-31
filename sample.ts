interface IUser {
  name: string;
  age: number;
}

function add(a: number, b: number): number {
  return a + b;
}

const name: string = "Michał";
const age: number = add(10, 10);
const user: IUser = { name, age };

println(JSON.stringify(user));
println(`Current date is: ${new Date().toISOString()}`);

class Person {
  private name: string;

  constructor(name: string) {
    this.name = name;
  }

  public sayHello() {
    println(`Hello, ${this.name}`);
  }
}

const person = new Person("Dominika");

const id = setInterval(() => {
  println("Repeat");
}, 1000);

setTimeout(() => {
  println("Hello World");
}, 1000);

setTimeout(() => {
  clearInterval(id);
}, 10 * 1000);

