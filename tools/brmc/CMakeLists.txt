project(brmc)

add_executable(brmc
        main.cpp
        preview.cpp

        XImporter.cpp
        XImporter.hpp

        HL1Importer.cpp
        HL1Importer.hpp
)
target_link_libraries(brmc PRIVATE BRender::Full assimp::assimp)
