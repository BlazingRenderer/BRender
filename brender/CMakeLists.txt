set(BRENDER_FOLDER "Brender")
add_subdirectory(inc)
add_subdirectory(host)
add_subdirectory(fw)
add_subdirectory(fmt)
add_subdirectory(math)
add_subdirectory(pixelmap)
add_subdirectory(std)
add_subdirectory(v1db)

add_subdirectory(nulldev)
add_subdirectory(glrend)

##
# Top level "brender" interface library.
# Pulls in v1db and glrend.
##
add_library(brender INTERFACE)
target_link_libraries(brender INTERFACE v1db glrend)
