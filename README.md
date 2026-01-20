# Protothreads

([A fork of Adam Dunkel's Protothreads library](https://dunkels.com/adam/pt/))

Protothreads are extremely lightweight stackless threads designed for severely memory constrained systems such as small embedded systems or sensor network nodes.
Protothreads can be used with or without an underlying operating system.

Protothreads provide a blocking context on top of an event-driven system, without the overhead of per-thread stacks.
The purpose of protothreads is to implement sequential flow of control without complex state machines or full multi-threading.

## Main Features

- No machine specific code - the protothreads library is pure C
- Does not use error-prone functions such as longjmp()
- Very small RAM overhead - only two bytes per protothread
- Can be used with or without an OS
- Provides blocking wait without full multi-threading or stack-switching
- Freely available under a BSD-like open source license

## Example Applications

- Memory constrained systems
- Event-driven protocol stacks
- Small embedded systems
- Sensor network nodes

## License

The protothreads library is released under an open source BSD-style license that allows for both non-commercial and commercial usage.
The only requirement is that credit is given.

## Authors

The protothreads library was written by Adam Dunkels <adam@sics.se> with support from Oliver Schmidt <ol.sc@web.de>.
Learn more at http://www.sics.se/~adam/pt/

## Documentation

Full API documentation can be found in the `doc/` subdirectory.

## Example Programs

Three example programs illustrating the use of protothreads can be found in the `examples/` directory:

| File | Description |
|------|-------------|
| `example-small.c` | A small example showing how to use protothreads |
| `example-buffer.c` | The bounded buffer problem with protothreads |
| `example-codelock.c` | A code lock with simulated key input |

Build with the rest of the project:

```bash
mkdir build && cd build
cmake ..
make
./examples/example-buffer
```

Disable building examples:

```bash
cmake -DBUILD_EXAMPLES=OFF ..
```

## Testing

The library includes a comprehensive test suite using the [Unity](https://github.com/ThrowTheSwitch/Unity) test framework.

### Running Tests

```bash
mkdir build && cd build
cmake ..
make
ctest
```

For verbose output on failures:

```bash
ctest --output-on-failure
```

Run a specific test:

```bash
ctest -R pt_semaphore
```

### Test Coverage

| Test | Description |
|------|-------------|
| `pt_lifecycle` | PT_INIT, PT_BEGIN, PT_END, PT_EXIT, PT_RESTART, return values |
| `pt_waiting` | PT_WAIT_UNTIL, PT_WAIT_WHILE, PT_YIELD, PT_YIELD_UNTIL |
| `pt_scheduling` | PT_SCHEDULE, PT_SPAWN, PT_WAIT_THREAD, nested threads |
| `pt_semaphore` | PT_SEM_INIT, PT_SEM_WAIT, PT_SEM_SIGNAL, producer-consumer |
| `lc_switch` | Local continuations using switch/case (default) |
| `lc_addrlabels` | Local continuations using GCC computed goto |

### Disabling Tests

```bash
cmake -DBUILD_TESTING=OFF ..
```

## Usage

Protothreads is a header-only library.
Simply copy the header files to your project and include `pt.h`:

```c
#include "pt.h"
```

Alternatively, add it to your CMake project:

```cmake
include(FetchContent)
FetchContent_Declare(
    protothreads
    GIT_REPOSITORY https://github.com/IdahoAvionics/protothreads.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(protothreads)

add_executable(my_app main.c)
target_link_libraries(my_app PRIVATE protothreads)
```
