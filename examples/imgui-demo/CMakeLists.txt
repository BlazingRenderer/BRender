#
# Dear ImGui over BRender's 2D primitives: ImGui's own demo window, drawn a
# frame at a time. The library behind it is contrib/imgui.
#
project(imgui-demo CXX)

add_executable(imgui-demo main.cpp)

target_link_libraries(imgui-demo PRIVATE brender-imgui brdemo)
set_property(TARGET imgui-demo PROPERTY FOLDER "BRender/Examples")

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS imgui-demo DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
