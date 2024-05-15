##
# Add all core targets to the "Core" export, and
# configure their "include" and "ddi" filesets to be copied.
##

include(GNUInstallDirs)

install(TARGETS
        brender brender-ddi
        EXPORT Core
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )

install(TARGETS
        brender-inc brender-inc-ddi
        inc fmt fw host math nulldev pixelmap std v1db
        EXPORT Core
        FILE_SET include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/brender
        FILE_SET ddi DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/brender/ddi
        )


if (TARGET glrend)
    install(TARGETS glrend-headers
            EXPORT Core
            FILE_SET include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/brender/glrend
            )

    install(TARGETS glrend
            EXPORT Core
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            )
endif()

if (TARGET sdl2dev)
    install(TARGETS sdl2dev-headers
            EXPORT Core
            FILE_SET include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/brender/sdl2dev
            )

    install(TARGETS sdl2dev
            EXPORT Core
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            )
endif()

if (TARGET softrend)
    install(TARGETS softrend
            EXPORT Core
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            )
endif()

if (TARGET pentprim)
    install(TARGETS pentprim
            EXPORT Core
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            )
endif()


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
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/BRenderConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/BRenderConfigVersion.cmake
        DESTINATION lib/cmake/brender
        )

# pkg-config

#get_target_property(COMPILE_DEFINITIONS brender-full INTERFACE_COMPILE_DEFINITIONS)

# message(FATAL_ERROR "XXXX: ${COMPILE_DEFINITIONS}")

file(GENERATE
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}-$<CONFIG>.pc"
        #INPUT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/brender.pc.in"
        CONTENT "\
libdir=\"${CMAKE_INSTALL_FULL_LIBDIR}\"
includedir=\"${CMAKE_INSTALL_FULL_INCLUDEDIR}/brender\"

Name: ${PROJECT_NAME}
URL: ${CMAKE_PROJECT_HOMEPAGE_URL}
Version: $<TARGET_PROPERTY:brender,VERSION>
Cflags: -I\"\${includedir}\" -I\"\${includedir}/glrend\" -I\"\${includedir}/sdl2dev\" -D$<JOIN:$<TARGET_PROPERTY:brender-full,INTERFACE_COMPILE_DEFINITIONS>, -D>
")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/brender.pc.in ${CMAKE_CURRENT_BINARY_DIR}/brender.pc)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/brender.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
