# Hello in C++ FACTS example

For the simple command-line build, you should get away with (-g for debug):


## Compile
```bash
g++ *.cpp *.c
```

## Run all tests
```bash
./a.out
```

## Run some tests
```bash
./a.out --facts_include='*F'
```

## Skip some tests
```bash
./a.out --facts_exclude='*F'
```



