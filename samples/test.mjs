export function test1() {
  console.log("Test1 call");
  console.log(`Dirname: ${import.meta.dirname}`)
  console.log(`Filename: ${import.meta.filename}`);
  console.log(`Url: ${import.meta.url}`);
}

test1();
throw new Error('some err');
