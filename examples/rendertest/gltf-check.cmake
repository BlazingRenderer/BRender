# Load every checked-in glTF through gltfview, so a reader or writer change
# cannot quietly stop a file parsing. Driven by `cmake -P` from CMakeLists.txt,
# which passes GLTFVIEW and the directories to sweep in.
#
# GLTFVIEW_BENCH_FRAMES is set by the caller through the test environment; with
# it unset these render in an endless loop rather than returning.

foreach(required GLTFVIEW DIR1)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} was not passed to this script")
    endif()
endforeach()

set(files)
foreach(var IN ITEMS DIR1 DIR2)
    if(DEFINED ${var})
        file(GLOB found "${${var}}/*.gltf")
        list(APPEND files ${found})
    endif()
endforeach()

list(LENGTH files n)
if(n EQUAL 0)
    message(FATAL_ERROR "no .gltf files found in ${DIR1} ${DIR2}")
endif()

set(failures 0)
foreach(f IN LISTS files)
    execute_process(
        COMMAND "${GLTFVIEW}" --force-software --software-bpp 8 -w 320 -h 240 --no-stats "${f}"
        RESULT_VARIABLE rc
        OUTPUT_QUIET
        ERROR_QUIET
        TIMEOUT 60
    )
    # RESULT_VARIABLE is a string, and a timeout comes back as one rather than
    # as a number - so compare as text.
    if(NOT rc STREQUAL "0")
        message(WARNING "failed to load ${f} (${rc})")
        math(EXPR failures "${failures} + 1")
    endif()
endforeach()

if(NOT failures EQUAL 0)
    message(FATAL_ERROR "${failures} of ${n} glTF files did not load")
endif()

message(STATUS "glTF: ${n} files load")
