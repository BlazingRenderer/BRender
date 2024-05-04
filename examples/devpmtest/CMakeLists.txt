project(devpmtest)

add_executable(devpmtest
        main.c
)
target_link_libraries(devpmtest PRIVATE BRender::Full BRender::DDI)
set_property(TARGET devpmtest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET devpmtest PROPERTY FOLDER "BRender/Examples")

if(TARGET SDL2::SDL2main)
        target_link_libraries(devpmtest PRIVATE SDL2::SDL2main)
endif()

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS devpmtest DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
