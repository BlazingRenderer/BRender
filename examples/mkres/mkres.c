#include <stdio.h>
#include <brender.h>

br_pixelmap *mkres_make_checkerboard_pixelmap(const char *name);
br_material *mkres_make_checkerboard_material(const char *name, br_pixelmap *pm);
br_model *mkres_make_quad(const char *name);
br_model *mkres_make_cube(const char *name);

int main(int argc, char **argv)
{
    br_pixelmap *pm_checkerboard;
    br_material *mat_checkerboard;
    br_model    *mdl_quad, *mdl_cube;
    int ret = 1;

    BrBegin();

    if((pm_checkerboard = mkres_make_checkerboard_pixelmap("checkerboard.pix")) == NULL) {
        fprintf(stderr, "failed to allocate checkerboard pixelmap\n");
        goto done;
    }

    if((mat_checkerboard = mkres_make_checkerboard_material("checkerboard.mat", pm_checkerboard)) == NULL) {
        fprintf(stderr, "failed to allocate checkboard material\n");
        goto done;
    }

    if((mdl_quad = mkres_make_quad("quad.dat")) == NULL) {
        fprintf(stderr, "failed to allocate quad model\n");
        goto done;
    }

    if((mdl_cube = mkres_make_cube("cube.dat")) == NULL) {
        fprintf(stderr, "failed to cube model\n");
        goto done;
    }

    BrModelSave("cube.dat", mdl_cube);
    BrModelSave("quad.dat", mdl_quad);
    BrPixelmapSave("checkerboard.pix", pm_checkerboard);
    BrMaterialSave("checkerboard.mat", mat_checkerboard);

    ret = 0;
done:
    BrEnd();
    return ret;
}
