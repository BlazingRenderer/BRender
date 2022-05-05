/*
 * Table of exported names
 */
static char * namePointers_BRHOST1[] = { "_HostInfo" };

/*
 * Table of ordinals
 */
static br_uint_16 nameOrdinals_BRHOST1[] = { 0 };

/*
 * Table of ordinals
 */
static void * functionPointers_BRHOST1[] = { HostInfo /* Ordinal 0 */ };

static br_image Image_BRHOST1 = {
	{0},
	"BRHOST1",							/* Identifier				*/
	BR_IMG_RESIDENT,					/* Image type				*/
	0,									/* Reference count			*/

	1,									/* Ordinal base				*/
	BR_ASIZE(functionPointers_BRHOST1),	/* Number of functions		*/
	functionPointers_BRHOST1,			/* Table of functions		*/

	BR_ASIZE(namePointers_BRHOST1),		/* Number of names			*/
	namePointers_BRHOST1,				/* Table of names 			*/
	nameOrdinals_BRHOST1,				/* Table of name ordinals	*/
	
	0,									/* Number of imports		*/
	NULL,								/* Table of imports			*/

	0,									/* Number of sections		*/
	NULL,								/* Table of sections		*/

	NULL,								/* Type pointer				*/
};

