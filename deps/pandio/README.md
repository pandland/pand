> ⚠️ Early stage of development. Library is still shaping and API is unstable.

<img src="https://github.com/user-attachments/assets/cf70d4ac-9cef-4c66-8295-04946abdafd2" alt="banner" />

# Pandio

Simple library for non-blocking I/O operations. Developed mainly for my PandJS project (JavaScript runtime).

### Build

Make sure you have CMake installed.

```sh
mkdir build
cd build
# inside build dir
cmake ..
# compile with platform's default compiler
cmake --build .
```

### TODO:

- [x] Timers using heap data structure (works like `setTimeout`, `setInterval`).
- [x] Non-blocking IO for sockets using epoll/kqueue/iocp.
- [x] Basic networking abstraction to handle TCP.
- [x] Support Linux.
- [x] Support Windows.
- [x] Thread pool.
- [x] Support BSD/MacOS (tested only on FreeBSD).
- [ ] Signals.
- [ ] Fix tests and GitHub workflows.
- [ ] Async files operations.
- [ ] Better error handling and error reporting to the end-user.

## License

Distributed under the MIT License. See `LICENSE` for more information.
