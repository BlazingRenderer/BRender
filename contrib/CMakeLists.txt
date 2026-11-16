add_subdirectory(editorcam)

# Dear ImGui. Its only users are the examples, which instantiate the library
# themselves when this is off - the examples' gate on BRENDER_HAVE_SDL3 does not
# reach here.
if(TARGET SDL3::SDL3)
    add_subdirectory(imgui)
endif()

