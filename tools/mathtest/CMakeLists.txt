project(mathtest)

add_executable(mathtest main.cpp)
target_link_libraries(mathtest PRIVATE BRender::Core)
