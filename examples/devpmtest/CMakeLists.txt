project(devpmtest)

add_executable(devpmtest
        main.c
)
target_link_libraries(devpmtest PRIVATE BRender::Full BRender::DDI)
set_property(TARGET devpmtest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET devpmtest PROPERTY FOLDER "BRender/Examples")

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS devpmtest DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()

if (PSP)
	create_pbp_file(
		TARGET ${PROJECT_NAME}
		ICON_PATH NULL
		BACKGROUND_PATH NULL
		PREVIEW_PATH NULL
		TITLE ${PROJECT_NAME}
	)
endif ()

