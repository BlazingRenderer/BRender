#include <stdio.h>
#include <brender.h>

br_pixelmap *mkres_make_checkerboard_pixelmap(const char *name);
br_pixelmap *mkres_make_checkerboard8_pixelmap(const char *name);
br_pixelmap *mkres_make_checkerboard24_pixelmap(const char *name);
br_pixelmap *mkres_make_checkerboard32_pixelmap(const char *name);
br_material *mkres_make_checkerboard_material(const char *name, br_pixelmap *pm);
br_model    *mkres_make_quad(const char *name);
br_model    *mkres_make_cube(const char *name);
br_pixelmap *mkres_make_smtpe_bars(br_uint_8 type, br_int_16 width);

int main(int argc, char **argv)
{
    br_pixelmap *pm_checkerboard, *pm_checkerboard8, *pm_checkerboard24, *pm_checkerboard32;
    br_material *mat_checkerboard, *mat_checkerboard8, *mat_checkerboard24, *mat_checkerboard32;
    br_model    *mdl_quad, *mdl_cube;
    int          ret = 1;

    BrBegin();

    if((pm_checkerboard = mkres_make_checkerboard_pixelmap("checkerboard.pix")) == NULL) {
        fprintf(stderr, "failed to allocate checkerboard pixelmap\n");
        goto done;
    }

    if((pm_checkerboard8 = mkres_make_checkerboard8_pixelmap("checkerboard8.pix")) == NULL) {
        fprintf(stderr, "failed to allocate checkerboard8 pixelmap\n");
        goto done;
    }

    if((pm_checkerboard24 = mkres_make_checkerboard24_pixelmap("checkerboard24.pix")) == NULL) {
        fprintf(stderr, "failed to allocate checkerboard24 pixelmap\n");
        goto done;
    }

    if((pm_checkerboard32 = mkres_make_checkerboard32_pixelmap("checkerboard32.pix")) == NULL) {
        fprintf(stderr, "failed to allocate checkerboard32 pixelmap\n");
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

    if((mat_checkerboard24 = mkres_make_checkerboard_material("checkerboard24.mat", pm_checkerboard24)) == NULL) {
        fprintf(stderr, "failed to allocate checkboard24 material\n");
        goto done;
    }

    if((mat_checkerboard32 = mkres_make_checkerboard_material("checkerboard32.mat", pm_checkerboard32)) == NULL) {
        fprintf(stderr, "failed to allocate checkboard32 material\n");
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
    BrPixelmapSave("checkerboard24.pix", pm_checkerboard24);
    BrPixelmapSave("checkerboard32.pix", pm_checkerboard32);
    BrMaterialSave("checkerboard.mat", mat_checkerboard);
    BrMaterialSave("checkerboard8.mat", mat_checkerboard8);
    BrMaterialSave("checkerboard24.mat", mat_checkerboard24);
    BrMaterialSave("checkerboard32.mat", mat_checkerboard32);

    {
        const static struct {
            br_uint_8   type;
            const char *name;
            br_int_16   width;
        } smpte_formats[] = {
            {.type = BR_PMT_INDEX_8,   .name = "index_8",        .width = 672},
            {.type = BR_PMT_RGB_555,   .name = "rgb_555",        .width = 672},
            {.type = BR_PMT_RGB_565,   .name = "rgb_565",        .width = 672},
            {.type = BR_PMT_RGB_888,   .name = "rgb_888",        .width = 672},
            {.type = BR_PMT_RGBA_8888, .name = "rgba_8888",      .width = 672},
            {.type = BR_PMT_RGBX_888,  .name = "rgbx_888",       .width = 672},
            {.type = BR_PMT_BGR_555,   .name = "bgr_555",        .width = 672},
            {.type = BR_PMT_ARGB_4444, .name = "argb_4444",      .width = 672},

            {.type = BR_PMT_INDEX_8,   .name = "index_8_small",  .width = 128},
            {.type = BR_PMT_RGB_565,   .name = "rgb_565_small",  .width = 128},
            {.type = BR_PMT_RGB_888,   .name = "rgb_888_small",  .width = 128},
            {.type = BR_PMT_RGBX_888,  .name = "rgbx_888_small", .width = 128},
        };

        for(int i = 0; i < BR_ASIZE(smpte_formats); ++i) {
            br_pixelmap *smpte;

            if((smpte = mkres_make_smtpe_bars(smpte_formats[i].type, smpte_formats[i].width)) == NULL) {
                fprintf(stderr, "failed to create %s SMPTE pixelmap\n", smpte_formats[i].name);
                continue;
            }

            smpte->identifier = BrResSprintf(smpte, "smpte_type%02d_%s.pix", smpte_formats[i].type, smpte_formats[i].name);

            BrPixelmapSave(smpte->identifier, smpte);
        }
    }

    ret = 0;
done:
    BrEnd();
    return ret;
}
