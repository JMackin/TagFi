cmake_minimum_required(VERSION 3.26)
project(TagFI C)

set(CMAKE_C_STANDARD 17)

add_executable(TagFI hashlattice.c /usr/local/include/sodium.h
        tagfi.h
        tagfi.c
        lattice_works.c
        jlm_random.c
        jlm_random.h
        fiforms.c
        fiforms.h
        fidi_masks.h
        fidi_masks.c
        fi_lattice.c
        fi_lattice.h
        lattice_rsps.c
        lattice_rsps.h
        lattice_reqs.c
        lattice_reqs.h
        lattice_works.h
        lattice_signals.c
        lattice_signals.h
        profiling.c
        consts.c
        consts.h
        consts.c
        consts.h
        reply_tools.c
        reply_tools.h
        FiOps.h
        ops.c
        ops.h

)
include_directories(/usr/local/include/sodium)
target_link_libraries(TagFI sodium)
