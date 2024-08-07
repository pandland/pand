#!/bin/bash

LIBS_DIR="./deps"

V8_DIR="$LIBS_DIR/v8"
INCLUDE_DIR="$V8_DIR/include"
LIB_DIR="$V8_DIR/out.gn/x64.release.sample/obj"


SRC="core/main.cc"

CXXFLAGS="-std=c++20 -I$INCLUDE_DIR -I$LIBS_DIR/swcc/include -I$LIBS_DIR/luxio/src"
LDFLAGS="-L$LIB_DIR -L$LIBS_DIR/luxio/build -L$LIBS_DIR/swcc/target/release -lswcc -lluxio -lv8_libplatform -lv8_monolith -pthread -lrt -ldl -DV8_ENABLE_SANDBOX=1 -DV8_COMPRESS_POINTERS=1"

g++ $CXXFLAGS $SRC $LDFLAGS -o pand

if [ $? -eq 0 ]; then
  echo "Compiled successfully."
else
  echo "Compilation failed."
fi
