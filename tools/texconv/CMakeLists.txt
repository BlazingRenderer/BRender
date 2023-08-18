project(texconv)

add_executable(texconv
        ./brbhook.c
        ./cla.c
        ./cla.h
        ./convert.c
        ./convert.h
        ./lists.c
        ./lists.h
        ./loadpix.c
        ./loadpix.h
        ./log.c
        ./log.h
        ./mip.cpp
        ./mip.h
        ./palquant.c
        ./palquant.h
        ./pmaps.c
        ./pmaps.h
        ./quantize.h
        ./savepix.c
        ./savepix.h
        ./savetga.c
        ./savetga.h
        ./texconv.c
        ./texconv.h
)
target_compile_definitions(texconv PRIVATE -DDISABLE_VIEWING)
target_link_libraries(texconv PRIVATE BRender::Core)

if (NOT BRENDER_DISABLE_INSTALL)
	install(TARGETS texconv DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
