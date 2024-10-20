# CONTRIBUTING

Currently we do not have strict coding-style in the project.

### Getting Started

1. **Clone the Repository:** To get started, clone the repository locally:

```sh
git clone https://github.com/pandland/pand.git
cd pand
```


2. **Building the Project:** Make sure you have CMake installed. Build infrastructure is still in early stage, currently we rely on newest version of v8 - this is not what I want, but for now it's fine.

```sh
# Download v8 and build
cmake -P v8.cmake

# Build our actual project
mkdir build
cd build

cmake ..
cmake --build .
```

> `v8.cmake` script is not tested yet on fresh Linux installation.

3. **IDE configuration:** If you are using **Visual Studio Code** I recommend using `clangd` for autocompletion, because C/C++ extension from Microsoft is very slow to work on this project. I suggest newer version of `clangd`, especially on Debian/Ubuntu, you can try nightly version: https://apt.llvm.org/

### Code Guidelines

- **Don't** abuse `std::exceptions`. We use them only when reporting very critical errors that cannot be handled in a reasonable way. Do not throw them inside callbacks managed by `v8`, for example, microtasks.
- **Namespaces:** Use proper namespaces to avoid name conflicts, and make sure to use appropriate names that reflect the scope of the functions or classes. Inside project we have `pand::core` namespace.
- **Header Files:** Place declarations in `.h` files, and implementation in `.cc` files.
- **Include Guards:** Use `#pragma once` in header files to avoid multiple inclusions.
- **Commenting:** Do not abuse comments. Document code that may not be immediately obvious.

### Licensing

By contributing, you agree that your contributions will be licensed under the same open source license that covers the project.
