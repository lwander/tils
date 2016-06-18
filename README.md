# HTTP server written in C

The goal is to create a static HTTP library, with a couple of constraints.

1. Minimize memory allocations.

2. Minimize memory copies.

3. Non-blocking IO whenever possible.

4. Run 1 thread per CPU core.

5. Minimal connection overhead.

## Compiling

Requires GCC 4.4+, tested with 4.9.2

```
$ make       # to build
$ make clean # to remove artifacts & executable
```

## Running

```
$ ./tils [port number] # default port is 80
```
