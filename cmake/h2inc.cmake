function(make_h2inc TARGET SRC DST)
    # The include list must stay inline: the ';' inside $<JOIN:...> is a CMake
    # list separator, so a variable holding it is split before the generator
    # expression is evaluated.
    set(target_includes "$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>")

    get_filename_component(src_dir "${SRC}" DIRECTORY)
    get_filename_component(src_name "${SRC}" NAME)
    add_custom_command(
            OUTPUT              ${DST}
            COMMAND             ${BRENDER_H2INC_EXECUTABLE}
                                -nologo -G3 -Zp4 -w -c -WIN32
                                -D_WIN32 -D__VISUALC -D__H2INC__ -D_NO_PROTOTYPES
                                --brender-hack
                                "$<$<BOOL:${target_includes}>:/I$<JOIN:${target_includes},;/I>>"
                                -Fa${DST} ${src_name}
            WORKING_DIRECTORY   ${src_dir}
            MAIN_DEPENDENCY     ${SRC}
            DEPENDS             ${BRENDER_H2INC_TOOL}
            COMMAND_EXPAND_LISTS
            VERBATIM
    )
    return()
endfunction()
