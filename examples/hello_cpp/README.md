# Hello in C++ FACTS example

For the simple command-line build, you should get away with (-g for debug):


## Compile
The -g is the debug flag, and the *.c captures the facts.c file that must also be built as part of the program:
```sh
c++ -g -o hello *.cpp *.c
```

## Run all tests
```sh
./hello
```

## Run some tests
```sh
./hello --facts_include='*F'
```

## Skip some tests
```sh
./hello --facts_exclude='*F'
```
