# Changelog

## 1.3
- The return values from a protothread function can now be used to determine if the protothread is waiting, has yielded, exited, or ended.
- Additional example program added to the distribution: example-small.c.

## 1.2.1
- A small typo in example-buffer.c (thanks to a lot of people for reporting this).

## 1.2
- The PT_YIELDING() macro is now no longer needed.
- Bugfixes: the implementation of PT_SCHEDULE() was missing a parenthesis which caused one of the example programs (example-buffer.c) to fail. (Thanks to Leonardo Palozzi for finding and fixing it!)

# 1.1
- Added PT_YIELD() functionality that allows a protothread to yield the CPU. (Thanks to Glen Worstell for suggesting this).
- The examples should now compile under MS Windows as well. (Thanks to Oliver Schmidt).
- Bugfixes: PT_SPAWN() now properly takes two pt state structures as parameters.
