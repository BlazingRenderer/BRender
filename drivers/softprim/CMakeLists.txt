##
# softprim - the software rasteriser.
#
# A portable C/C++ port of the x86 assembly rasteriser that was
# drivers/pentprim/. It presents the same driver entry point, device identifier
# and primitive library object name, so nothing outside the driver changed when
# it took pentprim's place.
##

add_library(softprim
        device.c
        driver.c
        match.c
        object.c
        plib.c
        pstate.c
        sbuffer.c
        raster.cpp

        device.h
        drv.h
        drv_ip.h
        object.h
        plib.h
        pstate.h
        softprim.h
        sbuffer.h
        template.h
        timestmp.h

        softprim_axes.h
        softprim_blocks.h
)

##
# The block list.
#
# infogen.pl in its softprim mode projects each .ifg block onto the axis
# vocabulary in softprim_axes.h and emits one line per block, carrying the axis
# tuple (which names the kernel) and the metadata pentprim's matcher tests.
# SP_SPEC names the axis values softprim currently implements: a shape outside
# the spec is emitted as SOFTPRIM_REFUSED rather than SOFTPRIM_BLOCK, so it has
# no kernel and refuses the primitive that reaches it instead of being drawn
# wrongly. Refusing rather than omitting it is deliberate - a variant that is
# spelled as a match flag on a block ordered before its twin would otherwise
# fall through to the twin. Widening coverage means adding axis values here and
# kernels in raster.cpp together.
#
# The same generated list is expanded twice - once by match.c to build the
# matcher's ordered table, once by raster.cpp to define the kernels, which
# merge_blocks.pl filters to the blocks that have one - so a block cannot
# describe something its kernel does not implement.
#
# The order is the order the matcher tries the tables: pentprim's use_mmx
# selects the MMX table before the general one for a given output format, so a
# format's MMX table is listed before its general table. The line and point
# tables follow, and their order relative to the triangle tables does not
# matter: the matcher tests the topology before anything else, so a line only
# ever reaches the line tables. pentprim has no MMX line or point table - its
# MMX array points at the general one for those topologies.
#
# SP_SHADE_CONST_I_RGB/SP_SHADE_INTERP_I_RGB are in the spec, and the
# arbitrary-width shapes that carry them are emitted: prim_t24's RGB_888 pair
# and the z-sorted 15/16bpp pair. infogen.pl still refuses the power-of-two
# 15/16bpp ones by an explicit rule, because naming an axis value admits every
# shape that carries it and no kernel exists for those. SP_TEX_555/SP_TEX_565
# are in the spec for the same reason and with the same rule: the z-sorted
# arbitrary-width triangle is emitted and the rest of the shapes that sample a
# 555/565 map are refused. See infogen.pl's softprim_implemented().
##
set(SP_TABLES prim_t8 mmx_t15 prm_t15 mmx_t16 prm_t16 prim_t24 prim_l8 prim_p8 prim_l15 prim_p15 prim_l16 prim_p16 prim_l24 prim_p24)

set(SP_SPEC "softprim_implemented=")
string(APPEND SP_SPEC "SP_FMT_I8,SP_FMT_555,SP_FMT_565,SP_FMT_888,SP_TOP_TRI,SP_TOP_LINE,SP_TOP_POINT,")
string(APPEND SP_SPEC "SP_DEPTH_NONE,SP_DEPTH_ZW,")
string(APPEND SP_SPEC "SP_SHADE_NONE,SP_SHADE_CONST_I,SP_SHADE_INTERP_I,SP_SHADE_CONST_I_TABLE,SP_SHADE_INTERP_I_TABLE,SP_SHADE_CONST_RGB,SP_SHADE_INTERP_RGB,")
string(APPEND SP_SPEC "SP_SHADE_CONST_I_RGB,SP_SHADE_INTERP_I_RGB,")
string(APPEND SP_SPEC "SP_TEX_NONE,SP_TEX_I8,SP_TEX_555,SP_TEX_565,SP_TEX_RGB888,SP_ADDR_NONE,SP_ADDR_SHIFT,SP_ADDR_DIVIDE,")
string(APPEND SP_SPEC "SP_PERSP_AFFINE,SP_PERSP_CORRECT,")
string(APPEND SP_SPEC "SP_BLEND_NONE,SP_BLEND_INDEX,SP_BLEND_DECAL,SP_BLEND_SCREENDOOR,SP_FOG_NONE,SP_FOG_INDEX,SP_DITH_NONE,SP_DITH_MAP,SP_DITH_COLOUR")

set(SP_GENERATED)
foreach (tbl ${SP_TABLES})
    set(sp_out ${CMAKE_CURRENT_BINARY_DIR}/softprim_${tbl}.inc)
    add_custom_command(
            OUTPUT ${sp_out}
            COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/infogen.pl softprim ${SP_SPEC}
                    < ${CMAKE_CURRENT_SOURCE_DIR}/tables/${tbl}.ifg
                    > ${sp_out}
            MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/tables/${tbl}.ifg
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/infogen.pl
            COMMENT "Generating softprim block list from ${tbl}.ifg"
    )
    list(APPEND SP_GENERATED ${sp_out})
endforeach ()

#
# The two consumers, and why they differ.
#
# raster.cpp defines one kernel per distinct tuple, so its list is the blocks
# merged by kernel name. match.c needs every block in table order, because the
# matcher walks it taking the first block whose requirements the bound state
# meets - and two blocks can share a kernel while disagreeing on their
# requirements (the MMX and general tables do), which is the fall-through this
# reproduces. So merge_blocks.pl writes both tables.
#
set(sp_kernels ${CMAKE_CURRENT_BINARY_DIR}/softprim_blocks_all.inc)
set(sp_matchers ${CMAKE_CURRENT_BINARY_DIR}/softprim_matchers.inc)
add_custom_command(
        OUTPUT ${sp_kernels} ${sp_matchers}
        COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/merge_blocks.pl ${sp_kernels} ${sp_matchers} ${SP_GENERATED}
        DEPENDS ${SP_GENERATED} ${CMAKE_CURRENT_SOURCE_DIR}/merge_blocks.pl
        COMMENT "Merging softprim block lists"
)

#
# Both consumers include a generated table, so both must be rebuilt when it
# changes. Object dependencies are what makes the custom command run at all.
#
set_property(SOURCE match.c APPEND PROPERTY OBJECT_DEPENDS ${sp_matchers})
set_property(SOURCE raster.cpp APPEND PROPERTY OBJECT_DEPENDS ${sp_kernels})
add_custom_target(softprim_blocks DEPENDS ${sp_kernels} ${sp_matchers})
add_dependencies(softprim softprim_blocks)

#
# The C++ here exists only for type safety; the library exposes a C ABI.
# Disable the runtime machinery that typing-only C++ does not need.
#
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set_source_files_properties(raster.cpp PROPERTIES
            COMPILE_OPTIONS "-fno-exceptions;-fno-rtti"
            )
endif ()

get_target_property(target_type softprim TYPE)
if (NOT target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(softprim PRIVATE DEFINE_BR_ENTRY_POINT)
endif ()

target_link_libraries(softprim PRIVATE BRender::DDI)
target_include_directories(softprim PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#
# match.c expands the generated block list twice, and the second expansion
# forwards __VA_ARGS__ from SOFTPRIM_BLOCK_* to SOFTPRIM_ENTRY. MSVC's traditional
# preprocessor does not unpack a forwarded variadic argument, so the parameters
# come out empty and the table does not compile. /std:c11 and /std:c17 are the
# only standards switches that turn on the conforming preprocessor - no C++
# standard does - so the C sources ask for one. The C++ sources want C++17 for
# `if constexpr`, which MSVC does not default to either.
#
set_target_properties(softprim PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE
        EXPORT_NAME "Drivers::SoftPrim"
        FOLDER "BRender/Drivers"
        OUTPUT_NAME "softprmf"
        )

add_library(BRender::Drivers::SoftPrim ALIAS softprim)
