const timers = Runtime.bind("timer");
const Timer = timers.Timer;

function setTimeout(cb, delay, ...args) {
  const timer = new Timer();

  timer.onTimeout = () => {
    cb(...args);
  }
  return timer.setTimeout(delay);
}

const id = setTimeout(() => {
  console.log("Callback :o");
}, 4000);

console.log(`timeout id: ${id}`);
