project(brmc)

add_executable(brmc main.cpp)
target_link_libraries(brmc PRIVATE BRender::Core assimp::assimp)
