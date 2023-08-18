##
# BRender public includes.
# Added to by each target.
##
add_library(brender-inc INTERFACE)
target_include_directories(brender-inc INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/brender>
        )

##
# BRender DDI - superset of the public includes.
# Added to by each target.
##
add_library(brender-inc-ddi INTERFACE)
target_link_libraries(brender-inc-ddi INTERFACE brender-inc)

set(BRENDER_TARGETS inc host fw fmt math pixelmap nulldev std v1db)
set(BRENDER_OBJECTS)

foreach (target ${BRENDER_TARGETS})
    add_subdirectory(${target})
    list(APPEND BRENDER_OBJECTS "$<TARGET_OBJECTS:${target}>")
endforeach ()

foreach (target ${BRENDER_TARGETS} inc-generated fw-generated)
    set_property(TARGET ${target} PROPERTY FOLDER "BRender/Core")
endforeach ()

##
# BRender "core" target.
# Linked to all subtargets.
##
add_library(brender)

##
# Link all the subtargets privately so our consumers don't inherit their
# includes, which pulls in DDI.
# Then INTERFACE-link brender-inc to get only the headers we want.
##
target_link_libraries(brender PRIVATE ${BRENDER_TARGETS})
target_link_libraries(brender INTERFACE brender-inc)
set_target_properties(brender PROPERTIES
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        FOLDER "BRender"
        EXPORT_NAME "Core" # To get BRender::Core when exported.
        )

# https://gitlab.kitware.com/cmake/cmake/-/issues/17356
if (BUILD_SHARED_LIBS)
    set_target_properties(${BRENDER_TARGETS} PROPERTIES
            POSITION_INDEPENDENT_CODE ON
            )
endif ()

##
# BRender DDI target.
# Same as Core, but exposes the DDI headers.
##
add_library(brender-ddi INTERFACE)
target_link_libraries(brender-ddi INTERFACE brender brender-inc-ddi)
set_target_properties(brender-ddi PROPERTIES
        EXPORT_NAME "DDI" # To get BRender::DDI when exported.
        )

##
# BRender "full" target, contains drivers too.
##
add_library(brender-full INTERFACE)
target_link_libraries(brender-full INTERFACE brender)
set_target_properties(brender-full PROPERTIES
        EXPORT_NAME "Full" # To get BRender::Full when exported.
        )
