interface IUser {
  name: string;
  age: number;
  skills: string[]
}

function add(a: number, b: number): number {
  return a + b;
}

const name: string = "Michał";
const skills: string[] = ["JavaScript", "C", "C++"];
const age: number = add(9, 11);
const user: IUser = { name, age, skills };

println(JSON.stringify(user));
println(`Current date is: ${new Date().toDateString()}`);

