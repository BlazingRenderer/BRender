project(pentprim C ASM_MASM)

include(h2inc)

make_h2inc(pentprim "${CMAKE_CURRENT_SOURCE_DIR}/drv.h" "${CMAKE_CURRENT_BINARY_DIR}/drv.inc")

set(USE_MMX ON)

set(ASM_FILES
	magicsym.asm

	safediv.asm
	sar16.asm
	decal.asm

	t_piza.asm
	ti8_piz.asm
	ti8_pizp.asm
	t15_pizp.asm
	tt15_piz.asm
	tt16_piz.asm
	tt24_piz.asm

	t_pia.asm
	ti8_pi.asm
	ti8_pip.asm
	t15_pip.asm
	tt15_pi.asm
	tt16_pi.asm
	tt24_pi.asm

	zb8.asm
	zb8sh.asm
	zb8p2lit.asm
	zb8p2fla.asm
	zb8p2ulb.asm
	zb8p2ltb.asm
	zb8p2flb.asm
	zb8p2unl.asm
	zb8awtm.asm
	zs8.asm
	zs8p2unl.asm
	zs8awtm.asm
	fpsetup.asm
	fti8_piz.asm

	zb16p2ul.asm

	pfpsetup.asm
	fti8pizp.asm
	fti8_pip.asm
)

set(INC_FILES
		586_macs.inc
		common.inc
		fpsetup.inc
		fpu.inc
		fpwork.inc
		iammx.inc
		locmacs.inc
		rastmacs.inc
		setupdat.inc
		unpack.inc
		xzrgb.inc
		xzrgbs.inc
		xzuv1.inc
		xzuvb.inc
		xzuvc.inc
		xzuvcb.inc
		xzuvrgb.inc
)

set_source_files_properties(${INC_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

set(INFOGEN ${CMAKE_CURRENT_SOURCE_DIR}/infogen.pl)

set(PRIM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/infogen.pl)

function(add_prim base)
	set(input_file "${CMAKE_CURRENT_SOURCE_DIR}/${base}.ifg")
	set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${base}.c")

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
	set(output_files "${CMAKE_CURRENT_BINARY_DIR}/${base}f.c" "${CMAKE_CURRENT_BINARY_DIR}/${base}x.c")

	set(args_float "")
	set(argx_fixed "")

	if(with_suffixes)
		list(APPEND args_float "image_suffix=f")
		list(APPEND args_fixed "image_suffix=x")
	endif()

	list(APPEND args_float "float_components")

	add_custom_command(
		OUTPUT ${output_files}
		COMMAND ${PERL_EXECUTABLE} ${INFOGEN} ${args_float} < ${input_file} > "${CMAKE_CURRENT_BINARY_DIR}/${base}f.c"
		COMMAND ${PERL_EXECUTABLE} ${INFOGEN} ${args_fixed} < ${input_file} > "${CMAKE_CURRENT_BINARY_DIR}/${base}x.c"
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
	list(APPEND ASM_FILES ./gsetuptx.asm)

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


set(masm_flags )

# -- The ASM_MASM compiler identification is unknown
# -- Found assembler: /run/current-system/sw/bin/uasm
if(CMAKE_ASM_MASM_COMPILER MATCHES ".*/uasm$")
	list(APPEND masm_flags "-q" "-Zne" "-6" "-Zp4")

	if(DJGPP)
		list(APPEND masm_flags "-djgpp")
		# list(APPEND masm_flags "-Zi8" "-Zd")
	elseif(MINGW)
		list(APPEND masm_flags "-coff")
	else()
		list(APPEND masm_flags "-elf" "-zcw")
	endif()
endif()

list(APPEND masm_flags "-safeseh")

message(STATUS "Assember Flags: ${masm_flags}")

set_source_files_properties(${ASM_FILES} PROPERTIES COMPILE_OPTIONS "${masm_flags}")

add_library(pentprim
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

		workspace.c
		workspace.h

		"${CMAKE_CURRENT_BINARY_DIR}/drv.inc"

		${ASM_FILES}
		${INC_FILES}
		${PRIM_FILES}
)

if(USE_MMX)
	target_compile_definitions(pentprim PRIVATE "-DUSE_MMX=1")
endif()

target_compile_definitions(pentprim PRIVATE "-DPARTS=0xffff")
set_source_files_properties(${ASM_FILES} PROPERTIES COMPILE_DEFINITIONS "PARTS=0ffffh")

if(DJGPP)
	# i586-pc-msdosdjgpp-gcc -E -dM - </dev/null | grep -i 'DOS\|GO32\|DJGPP' | sort -u
	set_source_files_properties(${ASM_FILES} PROPERTIES COMPILE_DEFINITIONS
			"__DJGPP=2"
			"__DJGPP__=2"
			"DJGPP=2"

			"__GO32=1"
			"__GO32__=1"
			"GO32=1"

			"__MSDOS=1"
			"__MSDOS__=1"
			"MSDOS=1"
	)
endif()

get_target_property(target_type pentprim TYPE)
if(NOT target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(pentprim PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif()

target_link_libraries(pentprim PRIVATE BRender::DDI)
target_include_directories(pentprim PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(pentprim PROPERTIES
		EXPORT_NAME "Drivers::SoftPrim"
		FOLDER "BRender/Drivers"
		OUTPUT_NAME "softprmf"
		)

add_library(BRender::Drivers::SoftPrim ALIAS pentprim)
