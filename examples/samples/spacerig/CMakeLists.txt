project(spacerig)

add_executable(spacerig
        spacerig.c
        stars.c
)
target_link_libraries(spacerig PRIVATE brdemo)
set_property(TARGET spacerig PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dat")
set_property(TARGET spacerig PROPERTY FOLDER "BRender/Examples/Samples")

set(SPACERIG_FILES airlockw.pix alcwallw.pix argohead.dat base_pgw.pix
        bulkheaw.pix corflorw.pix corwallw.pix cube.dat dungeonw.pix elevatow.pix
        energyw.pix floorw.pix florpipw.pix gridholw.pix gridw.pix hull.dat
        hull_up.dat leftdoow.pix lev_tagw.pix liftdoow.pix liftflor.dat logow.pix
        man.dat marblew.pix menuw.pix pagew.pix platform.dat powerrod.dat probe.dat
        rgtdoorw.pix roofgriw.pix roofw.pix room.dat ruffloow.pix spacerig.mat
        spotlite.dat starw.pix table.dat techwalw.pix texturew.pix title_hw.pix
        title_lw.pix tv.dat veinsw.pix vieww.pix wallmarw.pix wallmonw.pix
        walltexw.pix winpal.pal winsh8.tab winsh15.tab winsh16.tab
)

foreach(file ${SPACERIG_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS spacerig DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
