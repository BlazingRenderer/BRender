project(brdemo)

add_library(brdemo STATIC
        brdemo.h
        brdemo.c

        parg.c
        parg.h
)

target_link_libraries(brdemo PUBLIC BRender::Full)
target_include_directories(brdemo PUBLIC .)
set_property(TARGET brdemo PROPERTY FOLDER "BRender/Examples")

if(TARGET BRender::Drivers::SoftPrim)
    target_compile_definitions(brdemo PUBLIC "HAVE_SOFTPRIM=1")
endif()
