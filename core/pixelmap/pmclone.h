
#ifndef _PMCLONE_H_
#define _PMCLONE_H_

/* function typedefs */
typedef br_colour br_pixelmap_pixel_read_cbfn(const br_uint_8 *pixels, const br_pixelmap *pm);
typedef void      br_pixelmap_pixel_write_cbfn(br_uint_8 *pixels, br_colour colour);

/* pixelmap conversion structure */
typedef struct br_pixelmap_converter {

    /* pixel read function */
    br_pixelmap_pixel_read_cbfn *read;

    /* pixel write function */
    br_pixelmap_pixel_write_cbfn *write;

    /* pixelmap type */
    br_uint_32 type;

    /* pixelmap type string */
    const char *name;

} br_pixelmap_converter;

/* array of pixelmap converters */
extern br_pixelmap_converter br_pixelmap_converters[];

#endif /* _PMCLONE_H_ */
