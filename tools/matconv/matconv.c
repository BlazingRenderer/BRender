#include <brender.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define MAX_MATERIALS 256

enum {
    MODE_NONE   = 0,
    MODE_TO_MAT = 1,
    MODE_TO_SCR = 2,
};

static br_pixelmap *find_failed_hook(const char *name)
{
    br_pixelmap *pix = BrPixelmapAllocate(BR_PMT_RGB_888, 8, 8, NULL, BR_PMAF_NORMAL);
    pix->identifier  = BrResStrDup(pix, name);
    return pix;
}

int main(int argc, char **argv)
{
    int          mode = MODE_NONE;
    const char  *infile, *outfile;
    br_uint_32   num_read, num_written;
    br_material *material[MAX_MATERIALS];

    if(argc != 4) {
        fprintf(stderr, "Usage: %s <to-mat|to-scr> <input-file> <output-file>\n", argv[0]);
        return 0;
    }

    infile  = argv[2];
    outfile = argv[3];

    if(BrStrICmp(argv[1], "to-mat") == 0) {
        mode = MODE_TO_MAT;
    } else if(BrStrICmp(argv[1], "to-scr") == 0) {
        mode = MODE_TO_SCR;
    } else {
        fprintf(stderr, "Unknown mode: %s\n", argv[1]);
    }

    BrBegin();

    BrMapFindHook(find_failed_hook);
    BrTableFindHook(find_failed_hook);

    if(mode == MODE_TO_MAT)
        num_read = BrFmtScriptMaterialLoadMany(infile, material, MAX_MATERIALS);
    else
        num_read = BrMaterialLoadMany(infile, material, MAX_MATERIALS);

    if(num_read == 0) {
        fprintf(stderr, "Unable to read input file.\n");
        BrEnd();
        return 1;
    }

    if(mode == MODE_TO_MAT)
        num_written = BrMaterialSaveMany(outfile, material, (br_uint_16)num_read);
    else
        num_written = BrFmtScriptMaterialSaveMany(outfile, material, (br_uint_16)num_read);

    if(num_written == 0) {
        fprintf(stderr, "Unable to write output file.\n");
        BrEnd();
        return 0;
    }

    if(num_read != num_written) {
        fprintf(stderr, "WARNING: num read (%" PRIu32 ") != num written (%" PRIu32 ")\n", num_read, num_written);
    }

    BrEnd();
    return 0;
}
