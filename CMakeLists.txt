cmake_minimum_required(VERSION 3.26)
project(TagFI C)

set(CMAKE_C_STANDARD 17)

add_executable(TagFI hashlattice.c /usr/local/include/sodium.h
        lattice_nodes.h
        lattice_nodes.c
        lattice_works.c
        CryptOps.c
        CryptOps.h
        FiForms.c
        FiForms.h
        FiDiMasks.h
        FiDiMasks.c
        Lattice.c
        Lattice.h
        lattice_rsps.c
        lattice_rsps.h
        lattice_reqs.c
        lattice_reqs.h
        lattice_works.h
        lattice_signals.c
        lattice_signals.h
        Profiling.c
        Consts.c
        Consts.h
        Consts.c
        Consts.h
        RspOps.c
        RspOps.h
        FiOps.h
        Options.c
        Options.h
        lattice_sessions.c
        lattice_sessions.h
        Tools.c
        Tools.h
        FiOps.c

)
include_directories(/usr/local/include/sodium)
target_link_libraries(TagFI sodium)
