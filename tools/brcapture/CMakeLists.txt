project(brcapture)

add_library(brcapture STATIC
        capture.c
        brcapture.h
)

target_link_libraries(brcapture PUBLIC BRender::Full)
# ObjectQuery(), for the colour buffer's GL texture. Private, so that
# brcapture.h and this target's interface stay free of the DDI.
target_link_libraries(brcapture PRIVATE BRender::DDI)
target_include_directories(brcapture PUBLIC .)
set_property(TARGET brcapture PROPERTY FOLDER "BRender/Tools")
