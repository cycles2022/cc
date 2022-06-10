networkit_add_module(cycles
        ChordlessCycles.cpp
        ChordlessCyclesDias.cpp
        ChordlessCyclesUno.cpp
        ChordlessCyclesUno3.cpp
        ChordlessCyclesUno4.cpp
        ChordlessCyclesUno41.cpp
        PreprocessingForCylces.cpp
        )

networkit_module_link_modules(cycles
        auxiliary base centrality graph)

add_subdirectory(test)

add_executable(benchmarkCycles
        main.cpp)

target_link_libraries(benchmarkCycles
        networkit
        stdc++fs)

