cmake_minimum_required(VERSION 3.26)
project(TagFI C)

set(CMAKE_C_STANDARD 17)

add_executable(TagFI test_tagfiles.c /usr/local/include/sodium.h
        tagfi.h
        tagfi.c
        chkmk_didmap.c
        chkmk_didmap.h
        jlm_random.c
        jlm_random.h
        fiforms.c
        fiforms.h
        fidi_masks.h
        fidi_masks.c
        fi_lattice.c
        fi_lattice.h
        lattice_cmds.c
        lattice_cmds.h
)
include_directories(/usr/local/include/sodium)
target_link_libraries(TagFI sodium)
