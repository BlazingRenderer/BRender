function(make_h2inc TARGET SRC DST)
    set(mkdrv "${CMAKE_SOURCE_DIR}/contrib/mkdrv.py")
    set(target_includes "$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>")

    add_custom_command(
            OUTPUT          ${DST}
            COMMAND         python ${mkdrv} "$<$<BOOL:${target_includes}>:/I$<JOIN:${target_includes},;/I>>" ${SRC} ${DST}
            MAIN_DEPENDENCY ${SRC}
            DEPENDS         ${mkdrv}
            COMMAND_EXPAND_LISTS
            VERBATIM
    )
endfunction()
