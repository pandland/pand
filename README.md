<p align="center">
  <a href="https://pandland.github.io"><img src="https://github.com/user-attachments/assets/40c000fa-26b2-425d-98d0-ad68d3026b0e" alt="Logo" height=170></a>
</p>

<h1 align="center">PandJS</h1>

<div align="center">
  <a href="https://pandland.github.io/guides/getting-started">Documentation</a>
  <span>&nbsp;&nbsp;•&nbsp;&nbsp;</span>
  <a href="https://pandland.github.io">Website</a>
  <span>&nbsp;&nbsp;•&nbsp;&nbsp;</span>
  <a href="https://github.com/pandland/pand/issues/new">Issues</a>
  <br />
  <br />
</div>

> ⚠️ Early stage of development.

Simple, free and open-source JavaScript runtime environment. Everything under single executable called `pand`.

### Install

Very early builds are available on **Linux only**

```sh
curl https://pandland.github.io/install | sh
```

### Clone

```sh
git clone https://github.com/pandland/pand.git
```

### Building

Make sure you have CMake and python3 installed. Build infrastructure is still in **early stage**.

You need to download v8 as dependency and it will take reasonable amount of time to compile (even 40 mins - depends on machine).


```sh
python3 tools/build_v8.py
# Build our actual project
mkdir build
cd build

cmake ..
cmake --build .
```

> `v8.cmake` script is not tested yet on fresh Linux installation.

## License

Distributed under the MIT License. See `LICENSE` for more information.
