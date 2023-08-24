
#include <string.h>

#include "brender.h"
#include "dosio.h"

static struct {

	/* pixelmaps */
	br_pixelmap *screen;
	br_pixelmap *palette;

	/* initialized flag */
	br_boolean initialized;

	/* setup string */
	char *setup_string;
	char *driver;
	br_int_32 width;
	br_int_32 height;
	br_int_32 bpp;

} dosio;

/*
 * Setup/teardown
 */

br_error BR_PUBLIC_ENTRY DOSBegin(void)
{
	return BRE_OK;
}

void BR_PUBLIC_ENTRY DOSEnd(void)
{

}

/*
 * Graphics mode
 */

br_pixelmap *BR_PUBLIC_ENTRY DOSGfxBegin(const char *setup_string)
{
	/*
	 * just to be safe
	 */

	if (dosio.initialized)
		return NULL;
	else
		BrMemSet(&dosio, 0, sizeof(dosio));

	/*
	 * tokenize setup string
	 */
	if (setup_string)
	{
		const char delim[] = ",";
		char *token;

		token = strtok(setup_string, delim);

		while (token != NULL)
		{
			char *temp;

			/*
			 * get driver string, width, height, bpp
			 */
			if (BrStrNICmp(token, "VESA", 4) == 0 || BrStrNICmp(token, "MCGA", 4) == 0)
			{
				dosio.driver = BrResStrDup(NULL, token);
			}
			else if (BrStrNICmp(token, "B:", 2) == 0)
			{
				dosio.bpp = BrStrToL(token + 2, token + BrStrLen(token), 10);
			}
			else if (BrStrNICmp(token, "W:", 2) == 0)
			{
				dosio.width = BrStrToL(token + 2, token + BrStrLen(token), 10);
			}
			else if (BrStrNICmp(token, "H:", 2) == 0)
			{
				dosio.height = BrStrToL(token + 2, token + BrStrLen(token), 10);
			}

			token = strtok(NULL, delim);
		}

		dosio.setup_string = BrResStrDup(NULL, setup_string);
	}
	else
	{
		dosio.setup_string = BrResStrDup(NULL, "MCGA,W:320,H:200,B:8");
	}

	/*
	 * set defaults if the setup string didnt include them
	 */
	if (dosio.driver == NULL)
		dosio.driver = BrResStrDup(NULL, "MCGA");
	if (!dosio.bpp)
		dosio.bpp = 32;
	if (!dosio.width)
		dosio.width = 320;
	if (!dosio.height)
		dosio.height = 200;

	/*
	 * allocate a palette
	 */
	dosio.palette = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL);

	/*
	 * set initialized
	 */
	dosio.initialized = BR_TRUE;

	/*
	 * return screen buffer
	 */
	return dosio.screen;
}

void BR_PUBLIC_ENTRY DOSGfxEnd(void)
{
	/*
	 * sanity check
	 */
	if (!dosio.initialized)
		return;

	/*
	 * free strings
	 */
	BrResFree(dosio.driver);
	BrResFree(dosio.setup_string);

	/*
	 * end renderer
	 */
	BrRendererEnd();

	/*
	 * free pixelmaps
	 */
	BrPixelmapFree(dosio.screen);
	BrPixelmapFree(dosio.palette);
}

void BR_PUBLIC_ENTRY DOSGfxPaletteSet(br_pixelmap *pm)
{
	if (pm->type != BR_PMT_RGBX_888 && pm->type != BR_PMT_RGBA_8888)
		return;

	BrPixelmapCopy(dosio.palette, pm);
}

void BR_PUBLIC_ENTRY DOSGfxPaletteSetEntry(int i, br_colour colour)
{
	BrPixelmapPixelSet(dosio.palette, 0, i - dosio.palette->origin_y, colour);
}

/*
 * Mouse reading
 */

br_error BR_PUBLIC_ENTRY DOSMouseBegin(void)
{
	return BRE_OK;
}

void BR_PUBLIC_ENTRY DOSMouseEnd(void)
{

}

br_error BR_PUBLIC_ENTRY DOSMouseRead(br_int_32 *mouse_x, br_int_32 *mouse_y, br_uint_32 *mouse_buttons)
{
	return BRE_OK;
}

/*
 * Reading system clock
 */

br_error BR_ASM_CALL DOSClockBegin(void)
{
	return BRE_OK;
}

br_error BR_ASM_CALL DOSClockEnd(void)
{
	return BRE_OK;
}

br_uint_32 BR_ASM_CALL DOSClockRead(void)
{
	return BRE_OK;
}

/*
 * Keyboard UP/DOWN handling
 */

br_error BR_PUBLIC_ENTRY DOSKeyBegin(void)
{
	return BRE_OK;
}

void BR_PUBLIC_ENTRY DOSKeyEnd(void)
{

}

br_boolean BR_PUBLIC_ENTRY DOSKeyTest(br_uint_8 scancode, br_uint_8 qualifiers, br_uint_8 repeats)
{
	return BR_FALSE;
}

br_error BR_PUBLIC_ENTRY DOSKeyEnableBIOS(br_boolean flag)
{
	return BRE_OK;
}

/*
 * Divide overflow suppressor
 */

br_error BR_PUBLIC_ENTRY DOSDivTrapBegin(void)
{
	return BRE_OK;
}

void BR_PUBLIC_ENTRY DOSDivTrapEnd(void)
{

}

br_uint_32 BR_ASM_CALL DOSDivTrapCount(br_boolean reset)
{
	return 0;
}

/*
 * Event queue for mouse and keyboard
 */

br_error BR_PUBLIC_ENTRY DOSEventBegin(void)
{
	return BRE_OK;
}

void BR_PUBLIC_ENTRY DOSEventEnd(void)
{

}

br_boolean BR_PUBLIC_ENTRY DOSEventWait(struct dosio_event *event, br_boolean block)
{
	return BR_TRUE;
}
