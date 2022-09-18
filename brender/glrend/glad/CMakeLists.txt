project(glrend-glad)


#add_library(glrend-glad-headers INTERFACE)
#target_include_directories(glrend-glad-headers INTERFACE ./glad/include)

add_library(glrend-glad STATIC
        ./src/glad.c
        ./include/glad/glad.h
        ./include/KHR/khrplatform.h
)
target_include_directories(glrend-glad PUBLIC ./include)
