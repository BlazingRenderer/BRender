project(parallax)

add_executable(parallax
        parallax.c

        ../../contrib/editorcam/editorcam.c
        ../../contrib/editorcam/editorcam.h
        ../../contrib/editorcam/editorcam_p.h
)

target_include_directories(parallax PRIVATE ../../contrib/editorcam)

target_link_libraries(parallax PRIVATE brdemo)
set_property(TARGET parallax PROPERTY FOLDER "BRender/Examples")

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS parallax DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
