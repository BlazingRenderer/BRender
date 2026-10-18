# Run `mkres scenes` into a scratch directory and compare it against the
# checked-in fixtures. Driven by `cmake -P` from CMakeLists.txt, which passes
# MKRES, DAT and WORK in.
#
# The comparison is by content, and in both directions: a fixture that the
# generator no longer produces is as broken as one it produces and nobody
# checked in.

foreach(required MKRES DAT WORK)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} was not passed to this script")
    endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

execute_process(
    COMMAND "${MKRES}" scenes
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
)

if(NOT rc EQUAL 0)
    message(FATAL_ERROR "`mkres scenes' failed with ${rc}\n${out}${err}")
endif()

file(GLOB generated "${WORK}/*")
file(GLOB checked_in "${DAT}/*")

list(LENGTH generated n_generated)
list(LENGTH checked_in n_checked_in)

foreach(gen IN LISTS generated)
    get_filename_component(name "${gen}" NAME)
    set(ref "${DAT}/${name}")

    if(NOT EXISTS "${ref}")
        message(FATAL_ERROR "${name} is generated but not checked in")
    endif()

    file(SHA256 "${gen}" gen_sum)
    file(SHA256 "${ref}" ref_sum)

    if(NOT gen_sum STREQUAL ref_sum)
        message(FATAL_ERROR
                "${name} differs from ${DAT}/${name}: the generator and the checked-in fixtures have diverged")
    endif()
endforeach()

foreach(ref IN LISTS checked_in)
    get_filename_component(name "${ref}" NAME)
    if(NOT EXISTS "${WORK}/${name}")
        message(FATAL_ERROR "${name} is checked in but the generator does not produce it")
    endif()
endforeach()

message(STATUS "fixtures: ${n_generated} generated, ${n_checked_in} checked in, all identical")
