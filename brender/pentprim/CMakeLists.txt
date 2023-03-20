project(pentprim C ASM_MASM)

include(h2inc)

make_h2inc(pentprim "${CMAKE_CURRENT_SOURCE_DIR}/drv.h" "${CMAKE_CURRENT_BINARY_DIR}/drv.inc")

set(USE_MMX ON)

set(ASM_FILES
        magicsym.asm
)

set(INFOGEN ${CMAKE_CURRENT_SOURCE_DIR}/infogen.pl)

set(PRIM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/infogen.pl)

function(add_prim base)
	set(input_file "${CMAKE_CURRENT_SOURCE_DIR}/${base}.ifg")
	set(output_file "${base}.c")

	add_custom_command(
		OUTPUT ${output_file}
		COMMAND ${PERL_EXECUTABLE} ${INFOGEN} < ${input_file} > ${output_file}
		MAIN_DEPENDENCY ${input_file}
		DEPENDS ${INFOGEN}
	)

	set(XXXX ${input_file} ${output_file})
	set(PRIM_FILES ${PRIM_FILES} ${XXXX} PARENT_SCOPE)
endfunction()

function(add_prim_suffixed base with_suffixes)
	set(input_file "${CMAKE_CURRENT_SOURCE_DIR}/${base}.ifg")
	set(output_files "${base}f.c" "${base}x.c")

	set(args_float "")
	set(argx_fixed "")

	if(with_suffixes)
		list(APPEND args_float "image_suffix=f")
		list(APPEND args_fixed "image_suffix=x")
	endif()

	list(APPEND args_float "float_components")

	add_custom_command(
		OUTPUT ${output_files}
		COMMAND ${PERL_EXECUTABLE} ${INFOGEN} ${args_float} < ${input_file} > "${base}f.c"
		COMMAND ${PERL_EXECUTABLE} ${INFOGEN} ${args_fixed} < ${input_file} > "${base}x.c"
		MAIN_DEPENDENCY ${input_file}
		DEPENDS ${INFOGEN}
	)

	set(XXXX ${input_file} ${output_files})
	set(PRIM_FILES ${PRIM_FILES} ${XXXX} PARENT_SCOPE)

	set_property(SOURCE match.c APPEND PROPERTY OBJECT_DEPENDS ${output_files})
endfunction()

add_prim(prim_l15 "")
add_prim(prim_l16 "")
add_prim(prim_l24 "")
add_prim(prim_l8  "")
add_prim(prim_p15 "")
add_prim(prim_p16 "")
add_prim(prim_p24 "")
add_prim(prim_p8  "")
add_prim(prim_t24 "")

add_prim_suffixed(prim_t8 TRUE)
add_prim_suffixed(prm_t15 TRUE)
add_prim_suffixed(prm_t16 TRUE)
add_prim_suffixed(mmx_t15 FALSE)
add_prim_suffixed(mmx_t16 FALSE)

set_source_files_properties(${PRIM_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

if(USE_MMX)
	list(APPEND ASM_FILES ./rastbuff.asm)
	list(APPEND ASM_FILES ./setupdat.asm)

	list(APPEND ASM_FILES ./gsetuptf.asm)
	# fixed
	#list(APPEND ASM_FILES ./gsetuptx.asm)

	list(APPEND ASM_FILES ./safediv.asm)
	list(APPEND ASM_FILES ./sar16.asm)
	list(APPEND ASM_FILES ./decal.asm)

	list(APPEND ASM_FILES ./t_piza.asm)
	list(APPEND ASM_FILES ./ti8_piz.asm)
	list(APPEND ASM_FILES ./ti8_pizp.asm)
	list(APPEND ASM_FILES ./t15_pizp.asm)
	list(APPEND ASM_FILES ./tt15_piz.asm)
	list(APPEND ASM_FILES ./tt16_piz.asm)
	list(APPEND ASM_FILES ./tt24_piz.asm)

	list(APPEND ASM_FILES ./t_pia.asm)
	list(APPEND ASM_FILES ./ti8_pi.asm)
	list(APPEND ASM_FILES ./ti8_pip.asm)
	list(APPEND ASM_FILES ./t15_pip.asm)
	list(APPEND ASM_FILES ./tt15_pi.asm)
	list(APPEND ASM_FILES ./tt16_pi.asm)
	list(APPEND ASM_FILES ./tt24_pi.asm)

	list(APPEND ASM_FILES ./zb8.asm)
	list(APPEND ASM_FILES ./zb8sh.asm)
	list(APPEND ASM_FILES ./zb8p2lit.asm)
	list(APPEND ASM_FILES ./zb8p2fla.asm)
	list(APPEND ASM_FILES ./zb8p2ulb.asm)
	list(APPEND ASM_FILES ./zb8p2ltb.asm)
	list(APPEND ASM_FILES ./zb8p2flb.asm)
	list(APPEND ASM_FILES ./zb8p2unl.asm)
	list(APPEND ASM_FILES ./zb8awtm.asm)
	list(APPEND ASM_FILES ./zs8.asm)
	list(APPEND ASM_FILES ./zs8p2unl.asm)
	list(APPEND ASM_FILES ./zs8awtm.asm)
	list(APPEND ASM_FILES ./fpsetup.asm)
	list(APPEND ASM_FILES ./fti8_piz.asm)

	list(APPEND ASM_FILES ./zb16p2ul.asm)

	list(APPEND ASM_FILES ./pfpsetup.asm)
	list(APPEND ASM_FILES ./fti8pizp.asm)
	list(APPEND ASM_FILES ./fti8_pip.asm)

	list(APPEND ASM_FILES ./rastrise.asm)
	list(APPEND ASM_FILES ./xzuv.asm)
	list(APPEND ASM_FILES ./xzuvc.asm)
	list(APPEND ASM_FILES ./xzrgb.asm)
	list(APPEND ASM_FILES ./xzuvrgb.asm)
	list(APPEND ASM_FILES ./xzuv555.asm)
	list(APPEND ASM_FILES ./xzuvc555.asm)
	list(APPEND ASM_FILES ./xzrgb555.asm)
	list(APPEND ASM_FILES ./xzuvrgb5.asm)
endif()



set_source_files_properties(${ASM_FILES} PROPERTIES COMPILE_OPTIONS "-safeseh")

add_library(pentprim STATIC
        awtm.c
        awtmi.h
        awtmz.c
        device.c
        device.h
        dither.c
        driver.c
        drv.h
        drv_ip.h
        fperspi.h
        fperspzi.h
        frcp.c
        l_pi.c
        l_piz.c
        match.c
        match.h
        mmx_ip.h
        object.c
        object.h
        persp.c
        perspi.h
        perspz.c
        perspzi.h
        plib.c
        plib.h
        p_pi.c
        p_piz.c
        prim_ip.h
        pstate.c
        pstate.h
        rastparm.h
        rcp.c
        sbuffer.c
        sbuffer.h
        template.h
        timestmp.h
        work.c
        work.h

		"${CMAKE_CURRENT_BINARY_DIR}/drv.inc"

		${ASM_FILES}
		${PRIM_FILES}
)

if(USE_MMX)
	target_compile_definitions(pentprim PRIVATE "-DUSE_MMX=1")
endif()

target_compile_definitions(pentprim PRIVATE "-DPARTS=0xffff")
set_source_files_properties(${ASM_FILES} PROPERTIES COMPILE_DEFINITIONS "PARTS=0ffffh")

get_target_property(target_type pentprim TYPE)
if(NOT target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(pentprim PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif()

target_link_libraries(pentprim PRIVATE v1db)
target_include_directories(pentprim PRIVATE ../softrend) # FIXME
target_include_directories(pentprim PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_property(TARGET pentprim PROPERTY FOLDER ${BRENDER_FOLDER})
