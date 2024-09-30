# CONTRIBUTING

Currently we do not have strict coding-style in the project.

Just couple simple rules:

- seperate each function with two lines.
- put public declaration in headers inside `include` dir.
- put private/internal declarations in headers inside `src` dir.
- prefix public functions with `pd_` and private with `pd__` (use two underscores for private).
- use "unix" style lowercase function names (for example: NOT `SayHello` but `pd_say_hello`).

### Building

Project uses CMake for generating build files.

```sh
mkdir build
cd build

cmake ..
make
```

### Commits

Keep commits short (under 50 characters) and use [conventional-commits](https://www.conventionalcommits.org/en/v1.0.0/). For now commit scope is usually platform (`unix` or `win32`).

```sh
<type>[optional scope]: <description>
```

Example commit:

```sh
feat(win32): implement async file read
```
