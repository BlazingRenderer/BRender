function(make_h2inc TARGET SRC DST)
    set(mkdrv "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../contrib/mkdrv.pl")
    set(target_includes "$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>")

    set(env_args )
    if(WINE_EXECUTABLE)
        set(env_args "WINE_EXECUTABLE=${WINE_EXECUTABLE}")
    endif()

    add_custom_command(
            OUTPUT          ${DST}
            COMMAND         ${CMAKE_COMMAND} -E env ${env_args} -- ${PERL} ${mkdrv} "$<$<BOOL:${target_includes}>:/I$<JOIN:${target_includes},;/I>>" ${SRC} ${DST}
            MAIN_DEPENDENCY ${SRC}
            DEPENDS         ${mkdrv}
            COMMAND_EXPAND_LISTS
            VERBATIM
    )
endfunction()
