function(make_h2inc TARGET SRC DST)
    set(mkdrv "${CMAKE_SOURCE_DIR}/contrib/mkdrv.pl")
    set(target_includes "$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>")

    add_custom_command(
            OUTPUT          ${DST}
            COMMAND         /home/zane/Documents/Coding/h2incc/cmake-build-debug/h2incc -C "${CMAKE_CURRENT_SOURCE_DIR}/h2incc.ini" "$<$<BOOL:${target_includes}>:-I$<JOIN:${target_includes},;-I>>" -i ${SRC}
            #COMMAND         ${PERL} ${mkdrv} "$<$<BOOL:${target_includes}>:/I$<JOIN:${target_includes},;/I>>" ${SRC} ${DST}
            MAIN_DEPENDENCY ${SRC}
            DEPENDS         ${mkdrv}
            COMMAND_EXPAND_LISTS
            VERBATIM
    )
endfunction()
