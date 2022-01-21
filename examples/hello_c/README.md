# Hello in C FACTS example

For the simple command-line build, you should get away with (-g for debug):


## Compile
```bash
gcc -g -o hello *.c
```

## Run all tests
```bash
./hello
```

## Run some tests
```bash
./hello --facts_include='*F'
```

## Skip some tests
```bash
./hello --facts_exclude='*F'
```


