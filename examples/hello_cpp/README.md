# Hello in C++ FACTS example

For the simple command-line build, you should get away with (-g for debug):


## Compile
```sh
c++ -o hello *.cpp
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



