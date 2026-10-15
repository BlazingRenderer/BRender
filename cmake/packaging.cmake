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

install(TARGETS glrend-headers
        EXPORT Core
        FILE_SET include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/brender/glrend
        )

if (TARGET glrend)
    install(TARGETS glrend
            EXPORT Core
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            )
endif()

install(TARGETS glrend1x-headers
        EXPORT Core
        FILE_SET include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/brender/glrend1x
        )

if (TARGET glrend1x)
    install(TARGETS glrend1x
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

if (TARGET sdl3dev)
    install(TARGETS sdl3dev-headers
            EXPORT Core
            FILE_SET include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/brender/sdl3dev
            )

    install(TARGETS sdl3dev
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

if (TARGET softprim)
    install(TARGETS softprim
            EXPORT Core
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/brender
            )
endif()


# install(EXPORT) writes a relocatable _IMPORT_PREFIX (computed from the file's
# own location) only when the destination is relative; an absolute one pins it
# to the configure-time prefix instead. A packager that passes an absolute
# CMAKE_INSTALL_LIBDIR - Nix, with the libraries in their own output - also
# moves the package out from under that prefix, so fall back to a relative
# directory there.
if (IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
	set(BRENDER_CMAKE_INSTALL_DIR lib/cmake/brender)
else()
	set(BRENDER_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/brender")
endif()

install(EXPORT Core
        FILE BRenderTargets.cmake
        NAMESPACE BRender::
        DESTINATION ${BRENDER_CMAKE_INSTALL_DIR}
        )

include(CMakePackageConfigHelpers)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/BRenderConfig.cmake"
        INSTALL_DESTINATION ${BRENDER_CMAKE_INSTALL_DIR}
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
        DESTINATION ${BRENDER_CMAKE_INSTALL_DIR}
        )

# pkg-config
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/brender.pc.in ${CMAKE_CURRENT_BINARY_DIR}/brender.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/brender.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
foreach(driver sdl2dev sdl3dev glrend glrend1x softrend softprim mcga)
	if (TARGET ${driver})
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/brender-driver.pc.in ${CMAKE_CURRENT_BINARY_DIR}/brender-${driver}.pc @ONLY)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/brender-${driver}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
	endif()
endforeach()
