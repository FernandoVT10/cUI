#!/bin/bash
mkdir -p build

files="./src/main.c ./src/input.c ./src/cTooling.c ./src/keysManager.c"

gcc -Wall -Wextra -Werror -W -o ./build/main $files -I./raylib-5.5/include -L./raylib-5.5/lib/ -l:libraylib.a -lm -lcurl
