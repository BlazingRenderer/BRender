function(add_example name)
	add_executable(${name}
			${name}.c
			)
	target_link_libraries(${name} dosbase)
endfunction()

add_example(brtutor1)
add_example(brtutor2)
add_example(brtutor3)
add_example(brtutor4)
add_example(brtutor5)
add_example(brtutor5b)
add_example(brtutor6)
add_example(brtutor6b)
add_example(brtutor7)
add_example(brtutor7b)
add_example(brtutor8)
add_example(brtutor8b)
add_example(brtutor9)
add_example(brtutor10)

