# code

See [networkit README](https://github.com/CharJon/networkit#readme) for information on networkit.
Most relevant starting point for chordless cycle code is networkit/cpp/cycles/main.cpp.

## Clone & Build
- Clone this repo
- Run ```git submodule update --init --recursive```
- ```mkdir build && cd buld```
- ```cmake -DNETWORKIT_QUIET_LOGGING=ON ..``` (quiet for less console output)
- ```make -j 10```

The above steps should create the executable used in our experiments in build/networkit/cpp/cycles/benchmarkCycles
