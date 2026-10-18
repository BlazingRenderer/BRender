#
# The scene/fixture invariant: every scene the default set names has a fixture,
# and every fixture is named.
#
# A name with no file renders nothing and reports NO-REFERENCE, so the run looks
# merely unverified rather than broken; a file with no name is never rendered at
# all, so nothing asserts anything about it.
#
# The list comes from the tool's --list, not from parsing main.c: the binary is
# the definition of the default set, and reading the source couples the check to
# its spelling - an option whose name is "scene-dir" is indistinguishable from a
# scene called that.
#
# Inputs:
#   RENDERTEST  The binary to ask for its scene list.
#   DAT         The directory the fixtures live in.
#
cmake_minimum_required(VERSION 3.23)

foreach (var RENDERTEST DAT)
    if (NOT DEFINED ${var})
        message(FATAL_ERROR "${var} must be defined")
    endif ()
endforeach ()

execute_process(COMMAND "${RENDERTEST}" --list
                OUTPUT_VARIABLE named_text
                ERROR_VARIABLE  list_err
                RESULT_VARIABLE list_rc)

if (NOT list_rc EQUAL 0)
    message(FATAL_ERROR "`${RENDERTEST} --list' failed (${list_rc}): ${list_err}")
endif ()

# printf() ends lines with CRLF on Windows; fold that away before splitting.
string(REPLACE "\r" "" named_text "${named_text}")
string(REPLACE "\n" ";" named "${named_text}")
list(REMOVE_ITEM named "") # the trailing newline
list(SORT named)

file(GLOB fixture_paths "${DAT}/*.gltf")
set(fixtures)
foreach (path IN LISTS fixture_paths)
    get_filename_component(name "${path}" NAME_WE)
    list(APPEND fixtures "${name}")
endforeach ()
list(SORT fixtures)

set(missing)
foreach (scene IN LISTS named)
    if (NOT scene IN_LIST fixtures)
        list(APPEND missing "${scene}")
    endif ()
endforeach ()

set(unused)
foreach (fixture IN LISTS fixtures)
    if (NOT fixture IN_LIST named)
        list(APPEND unused "${fixture}")
    endif ()
endforeach ()

if (missing)
    list(JOIN missing "\n  " text)
    message(FATAL_ERROR "scene names with no fixture under ${DAT}:\n  ${text}")
endif ()

if (unused)
    list(JOIN unused "\n  " text)
    message(FATAL_ERROR "fixtures no default scene names, so nothing renders them:\n  ${text}")
endif ()

list(LENGTH named count)
message(STATUS "${count} scenes, all with fixtures; no unrendered fixtures")
