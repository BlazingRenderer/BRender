##
# Add all core targets to the "Core" export, and
# configure their "include" and "ddi" filesets to be copied.
##
install(TARGETS
        brender-inc brender-inc-ddi
        inc fmt fw host math nulldev pixelmap std v1db
        brender brender-ddi
        EXPORT Core
        FILE_SET include DESTINATION include
        FILE_SET ddi DESTINATION ddi
        )

if (BRENDER_BUILD_DRIVERS)
    if (TARGET glrend)
        install(TARGETS glrend-headers
                EXPORT Core
                FILE_SET include DESTINATION drivers/glrend/include
                )

        install(TARGETS glrend
                EXPORT Core
                )
    endif()

    if (TARGET sdl2dev)
        install(TARGETS sdl2dev-headers
                EXPORT Core
                FILE_SET include DESTINATION drivers/sdl2dev/include
                )

        install(TARGETS sdl2dev
                EXPORT Core
                )
    endif()

    if (TARGET softrend)
        install(TARGETS softrend
                EXPORT Core
                )
    endif()

    if (TARGET pentprim)
        install(TARGETS pentprim
                EXPORT Core
                )
    endif()

endif ()


install(EXPORT Core
        FILE BRenderTargets.cmake
        NAMESPACE BRender::
        DESTINATION lib/cmake/brender
        )

include(CMakePackageConfigHelpers)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/BRenderConfig.cmake"
        INSTALL_DESTINATION lib/cmake/brender
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/BRenderConfigVersion.cmake"
        VERSION "1.3.2"
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/BRenderConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/BRenderConfigVersion.cmake
        DESTINATION lib/cmake/brender
        )