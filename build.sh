#!/bin/bash

V8_DIR="./v8"
INCLUDE_DIR="$V8_DIR/include"
LIB_DIR="$V8_DIR/out.gn/x64.release.sample/obj"

SRC="main.cc"

CXXFLAGS="-std=c++20 -I$INCLUDE_DIR -I./swcc/include"
LDFLAGS="-L$LIB_DIR -L./swcc/target/release -lswcc -lv8_libplatform -lv8_monolith -pthread -lrt -ldl -DV8_ENABLE_SANDBOX=1 -DV8_COMPRESS_POINTERS=1"

g++ $CXXFLAGS $SRC $LDFLAGS -o sample

if [ $? -eq 0 ]; then
  echo "Compiled successfully."
else
  echo "Compilation failed."
fi
