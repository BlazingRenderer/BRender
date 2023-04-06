#include <brender.h>

typedef struct br_smpte_colours {
    br_colour grey40;
    br_colour white75;
    br_colour yellow75;
    br_colour cyan75;
    br_colour green75;
    br_colour magenta75;
    br_colour red75;
    br_colour blue75;
    br_colour black75;
    br_colour white100;
    br_colour plusQ;
    br_colour plusI;
    br_colour minusI;

    br_colour plugeL;
    br_colour plugeR;
} br_smpte_colours;

const static br_smpte_colours smpte_rgba_8888 = {
    .grey40    = BR_COLOUR_RGBA(104, 104, 104, 255),
    .white75   = BR_COLOUR_RGBA(180, 180, 180, 255),
    .yellow75  = BR_COLOUR_RGBA(180, 180, 16, 255),
    .cyan75    = BR_COLOUR_RGBA(16, 180, 180, 255),
    .green75   = BR_COLOUR_RGBA(16, 180, 16, 255),
    .magenta75 = BR_COLOUR_RGBA(180, 16, 180, 255),
    .red75     = BR_COLOUR_RGBA(180, 16, 16, 255),
    .blue75    = BR_COLOUR_RGBA(16, 16, 180, 255),
    .black75   = BR_COLOUR_RGBA(16, 16, 16, 255),
    .white100  = BR_COLOUR_RGBA(235, 235, 235, 255),
    .plusQ     = BR_COLOUR_RGBA(72, 16, 118, 255),
    .plusI     = BR_COLOUR_RGBA(106, 52, 16, 255),
    .minusI    = BR_COLOUR_RGBA(16, 70, 106, 255),
    .plugeL    = BR_COLOUR_RGBA(9, 9, 9, 255),
    .plugeR    = BR_COLOUR_RGBA(29, 29, 29, 255),
};

#define SMPTE_NUM_BARS       7
#define SMPTE_NUM_SQUARES    4
#define SMPTE_NUM_PLUGE_BARS 3

typedef struct br_smpte_region {
    br_rectangle rect;
    br_colour    colour;
} br_smpte_region;

static void fill_smpte(br_pixelmap *pm, const br_smpte_colours *colours)
{
    const br_colour row1_colours[SMPTE_NUM_BARS] = {
        colours->white75,   colours->yellow75, colours->cyan75, colours->green75,
        colours->magenta75, colours->red75,    colours->blue75,
    };
    const br_colour row2_colours[SMPTE_NUM_BARS] = {
        colours->blue75, colours->black75, colours->magenta75, colours->black75,
        colours->cyan75, colours->black75, colours->grey40,
    };
    const br_colour row3_square_colours[SMPTE_NUM_SQUARES] = {
        colours->minusI,
        colours->white100,
        colours->plusQ,
        colours->black75,
    };
    const br_colour pluge_colours[SMPTE_NUM_PLUGE_BARS] = {
        /* leftmost one with intensity 4% below black (super-black or blacker than black, 3.5 IRE) */
        colours->plugeL,
        /* a middle one with intensity exactly equal to black (7.5 IRE) */
        colours->black75,
        /* a rightmost one with intensity 4% above black level (11.5 IRE) */
        colours->plugeR,
    };

    br_smpte_region regions[SMPTE_NUM_BARS + SMPTE_NUM_BARS + 4 + 4];

    int bar_columns[SMPTE_NUM_BARS];
    int accum_x, accum_y;
    int row12_width = pm->width / SMPTE_NUM_BARS;

    /*
     * Calculate the rects for the vertical bars.
     */
    accum_x = 0;
    accum_y = 0;
    for(int i = 0; i < SMPTE_NUM_BARS; ++i) {
        bar_columns[i] = accum_x;

        regions[i].rect.x = accum_x;
        regions[i].rect.y = 0;
        regions[i].rect.w = row12_width;
        regions[i].rect.h = (int)((float)pm->height * (2.0f / 3.0f));
        regions[i].colour = row1_colours[i];

        regions[i + SMPTE_NUM_BARS].rect.x = regions[i].rect.x;
        regions[i + SMPTE_NUM_BARS].rect.y = regions[i].rect.h;
        regions[i + SMPTE_NUM_BARS].rect.w = regions[i].rect.w;
        regions[i + SMPTE_NUM_BARS].rect.h = pm->height / 12;
        regions[i + SMPTE_NUM_BARS].colour = row2_colours[i];

        if(i == SMPTE_NUM_BARS - 1) {
            regions[i].rect.w                  = pm->width - regions[i].rect.x;
            regions[i + SMPTE_NUM_BARS].rect.w = pm->width - regions[i + SMPTE_NUM_BARS].rect.x;
        }

        accum_x += regions[i].rect.w;
    }
    accum_y += regions[0].rect.h + regions[SMPTE_NUM_BARS].rect.h;

    /*
     * Calculate the castellations.
     */
    accum_x = 0;
    for(int i = 0; i < SMPTE_NUM_SQUARES; ++i) {
        br_smpte_region *r = regions + SMPTE_NUM_BARS + SMPTE_NUM_BARS + i;

        r->rect.x = accum_x;
        r->rect.y = accum_y;
        r->rect.w = ((pm->width / SMPTE_NUM_BARS) * 5) / SMPTE_NUM_SQUARES;
        r->rect.h = pm->height - accum_y;
        r->colour = row3_square_colours[i];
        accum_x += r->rect.w;
    }

    /*
     * Draw the pluge bars. They're directly under the red bar.
     * Extend the square slightly if the pixels don't align.
     */
    {
        br_smpte_region *r = regions + SMPTE_NUM_BARS + SMPTE_NUM_BARS + SMPTE_NUM_SQUARES - 1;
        r->rect.w          = regions[SMPTE_NUM_BARS + 5].rect.x - r->rect.x;
        accum_x            = r->rect.x + r->rect.w;
    }

    for(int i = 0; i < SMPTE_NUM_PLUGE_BARS; ++i) {
        br_smpte_region *r = regions + SMPTE_NUM_BARS + SMPTE_NUM_BARS + SMPTE_NUM_SQUARES + i;

        r->rect.x = accum_x;
        r->rect.y = regions[SMPTE_NUM_BARS + 5].rect.y + regions[SMPTE_NUM_BARS + 5].rect.h;
        r->rect.w = row12_width / 3;
        r->rect.h = pm->height - accum_y;
        r->colour = pluge_colours[i];
        accum_x += row12_width / 3;
    }

    {
        br_smpte_region *bb = regions + SMPTE_NUM_BARS - 1;
        br_smpte_region *r  = regions + SMPTE_NUM_BARS + SMPTE_NUM_BARS + SMPTE_NUM_SQUARES + SMPTE_NUM_PLUGE_BARS - 1;

        r->rect.w = bb->rect.x - r[-1].rect.x;
    }

    /*
     * Final region, the last black square. This is under the blue bar.
     */
    {
        br_smpte_region *bb = regions + SMPTE_NUM_BARS - 1;
        br_smpte_region *r  = regions + SMPTE_NUM_BARS + SMPTE_NUM_BARS + SMPTE_NUM_SQUARES + SMPTE_NUM_PLUGE_BARS;
        r->rect.x           = bb->rect.x;
        r->rect.y           = r[-1].rect.y;
        r->rect.w           = bb->rect.w;
        r->rect.h           = pm->height - accum_y;
        r->colour           = colours->black75;
    }

    for(int i = 0; i < BR_ASIZE(regions); ++i) {
        BrPixelmapRectangleFill(pm, regions[i].rect.x, regions[i].rect.y, regions[i].rect.w, regions[i].rect.h,
                                regions[i].colour);
    }
}

const static struct {
    br_uint_8               type;
    const br_smpte_colours *colours;
} colour_types[] = {
    {.type = BR_PMT_RGBA_8888, .colours = &smpte_rgba_8888},
    {.type = BR_PMT_RGBX_888,  .colours = &smpte_rgba_8888},
};

br_pixelmap *mkres_make_smtpe_bars(br_uint_8 type, br_int_16 width)
{
    br_pixelmap            *pm;
    br_int_16               height  = (br_int_16)floorf((float)width / 1.33333333333333333f);
    const br_smpte_colours *colours = NULL;

    for(size_t i = 0; i < BR_ASIZE(colour_types); ++i) {
        if(colour_types[i].type == type) {
            colours = colour_types[i].colours;
            break;
        }
    }

    if(colours == NULL)
        return NULL;

    pm             = BrPixelmapAllocate(type, width, height, NULL, BR_PMAF_NORMAL);
    pm->identifier = BrResStrDup(pm, "SMPTE");
    fill_smpte(pm, colours);
    return pm;
}
