project(inc)

target_include_directories(brender-inc INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
        )

target_compile_definitions(brender-inc INTERFACE
        BASED_FIXED=0
        BASED_FLOAT=1
        PARANOID=1
        )


# Generate pretok.h, pretok_c.h, and toktype.c
add_custom_command(
        OUTPUT pretok.h pretok.c toktype.c
        COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tokgen.pl" "${CMAKE_CURRENT_SOURCE_DIR}/pretok.tok"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/pretok.tok"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tokgen.pl"
)

set(GENERATED_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/pretok.h
        ${CMAKE_CURRENT_BINARY_DIR}/pretok.c
        ${CMAKE_CURRENT_BINARY_DIR}/toktype.c
        )

set(INC_TOKEN_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/pretok.tok
        ${CMAKE_CURRENT_SOURCE_DIR}/tokgen.pl
        )
source_group("Token" FILES ${INC_TOKEN_FILES} ${GENERATED_FILES})
set_source_files_properties(${INC_TOKEN_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

add_custom_target(inc-generated DEPENDS ${GENERATED_FILES})

set(DDI_FILES
        )
source_group("DDI" FILES ${DDI_FILES})

set(INC_FILES
        actor.h
        angles.h
        assoca_p.h
        assocarr.h
        brdiag.h
        brender.h
        brerror.h
        brfile.h
        brfont.h
        brlimits.h
        brlog.h
        brmem.h
        brv1db.h
        brvector.h
        camera.h
        colour.h
        compend.h
        compiler.h
        custsupt.h
        dmyifix.h
        fixed.h
        fw_p.h
        light.h
        material.h
        math_p.h
        matrix.h
        model.h
        order.h
        pixelmap.h
        pm_p.h
        pool.h
        quat.h
        render.h
        scalar.h
        syscn_p.h
        token.h
        transfrm.h
        v1db_p.h
        ${CMAKE_CURRENT_BINARY_DIR}/pretok.h
        )

add_library(inc OBJECT
        dummy.c
        ${INC_TOKEN_FILES}
        )

target_sources(inc INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
        FILES ${INC_FILES}
        )

target_include_directories(inc PRIVATE .)
target_link_libraries(inc PRIVATE brender-inc-ddi)

add_dependencies(brender-inc INTERFACE inc-generated)
