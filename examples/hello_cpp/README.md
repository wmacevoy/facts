# Hello in C++ FACTS example

For the simple command-line build, you should get away with (turn on debug with -g  and use the math librray with -lm):


## Compile
```bash
g++ -g *.cpp *.c -lm
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



