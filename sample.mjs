const set = new Set();
set.add(set);
set.add(40);

const arr = [];
const arr2 = [1, 2, 3, [22, [22, 400, 100, [20,[40, { id: 20 }]]]]];
arr[0] = undefined;
arr[1] = "heheszki"
arr[2] = arr;
arr[3] = 0
arr[4] = null;
arr[5] = arr2;
arr[7] = 2;

//console.log(format([arr]));
console.log(arr);
