project(pixelmap)

add_library(v1db-headers INTERFACE)
target_include_directories(v1db-headers INTERFACE .)
target_link_libraries(v1db-headers INTERFACE fw-headers)
target_link_libraries(v1db-headers INTERFACE math-headers)

set(V1DB_FILES
	./actsupt.c
	./custsupt.c
	./dbquery.c
	./dbsetup.c
	./def_mat.c
	./def_mdl.c
	./def_otab.c
	./enables.c
	./ffhooks.c
	./matsupt.c
	./modrend.c
	./modsupt.c
	./otable.c
	./pick.c
	./prelight.c
	./prepmap.c
	./prepmatl.c
	./prepmesh.c
	./preptab.c
	./regsupt.c
	./render.c
	./v1dbfile.c

	./v1db.h
	./v1db_ip.h
	./zsmacro.h
)

add_library(v1db STATIC ${V1DB_FILES})
target_link_libraries(v1db v1db-headers)
target_link_libraries(v1db host)
target_link_libraries(v1db fmt)
target_link_libraries(v1db pixelmap)
target_link_libraries(v1db fw)
target_link_libraries(v1db math)
target_link_libraries(v1db std)

set_property(TARGET v1db PROPERTY FOLDER ${BRENDER_FOLDER})

