#define NUM_STARS 2001

typedef struct {
   br_vector4  coords;
   br_uint_32  colour;
   char *name;
} STAR,*STARPTR ;


void StarfieldDraw(br_pixelmap *window,
		  br_actor *world,
		  br_actor *camera,
		  br_pixelmap *shade,
		  br_pixelmap *sprite,
		  int names);


void SetupStarfield();
