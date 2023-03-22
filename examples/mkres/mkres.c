#include <stdio.h>
#include <brender.h>

br_pixelmap *mkres_make_checkerboard_pixelmap(const char *name);
br_pixelmap *mkres_make_checkerboard8_pixelmap(const char *name);
br_material *mkres_make_checkerboard_material(const char *name, br_pixelmap *pm);
br_model *mkres_make_quad(const char *name);
br_model *mkres_make_cube(const char *name);

int main(int argc, char **argv)
{
    br_pixelmap *pm_checkerboard, *pm_checkerboard8;
    br_material *mat_checkerboard, *mat_checkerboard8;
    br_model    *mdl_quad, *mdl_cube;
    int ret = 1;

    BrBegin();

    if((pm_checkerboard = mkres_make_checkerboard_pixelmap("checkerboard.pix")) == NULL) {
        fprintf(stderr, "failed to allocate checkerboard pixelmap\n");
        goto done;
    }

    if((pm_checkerboard8 = mkres_make_checkerboard8_pixelmap("checkerboard8.pix")) == NULL) {
        fprintf(stderr, "failed to allocate checkerboard8 pixelmap\n");
        goto done;
    }

    if((mat_checkerboard = mkres_make_checkerboard_material("checkerboard.mat", pm_checkerboard)) == NULL) {
        fprintf(stderr, "failed to allocate checkboard material\n");
        goto done;
    }

    if((mat_checkerboard8 = mkres_make_checkerboard_material("checkerboard8.mat", pm_checkerboard8)) == NULL) {
        fprintf(stderr, "failed to allocate checkboard8 material\n");
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
    BrPixelmapSave("checkerboard8.pix", pm_checkerboard8);
    BrMaterialSave("checkerboard.mat", mat_checkerboard);
    BrMaterialSave("checkerboard8.mat", mat_checkerboard8);

    ret = 0;
done:
    BrEnd();
    return ret;
}
