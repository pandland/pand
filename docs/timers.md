## Timer functions

You can schedule specific callbacks using timer functions that are available globally.

> It's worth noting that the pool of IDs used by setTimeout() and setInterval() are shared, which means you can technically use clearTimeout() and clearInterval() interchangeably. However, for clarity, you should avoid doing so.

### `setTimeout(callback: function, delay: number): number`

Sets a timer which executes a callback function once the timer expires. Returns a timer id.

### `clearTimeout(id: number)`

Cancels a timeout previously established by calling `setTimeout()`. Takes a timer id as parameter.

> Passing an invalid id to clearTimeout() silently does nothing; no exception is thrown.

### `setInterval(callback: function, delay: number): number`

Repeatedly calls a callback function, with a fixed time delay between each call.

### `clearInterval(id: number)`

Cancels a timeout previously established by calling `clearInterval()`. Takes a timer id as parameter.

> Passing an invalid id to clearInterval() silently does nothing; no exception is thrown.
