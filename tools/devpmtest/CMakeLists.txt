project(devpmtest)

add_executable(devpmtest
        main.c
        bench.c
        bench.h
)
target_link_libraries(devpmtest PRIVATE BRender::Full BRender::DDI parg brcapture)
set_property(TARGET devpmtest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/examples/dat")
set_property(TARGET devpmtest PROPERTY FOLDER "BRender/Tools")

if(TARGET SDL3::SDL3)
    target_compile_options(devpmtest PRIVATE -DUSE_SDL2=0 -DUSE_SDL3=1)
elseif(TARGET SDL2::SDL2)
    target_compile_options(devpmtest PRIVATE -DUSE_SDL2=1 -DUSE_SDL3=0)
endif()

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS devpmtest DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
