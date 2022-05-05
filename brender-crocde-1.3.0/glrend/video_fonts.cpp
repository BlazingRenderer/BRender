#include <log.h>
#include <brender.h>
#include "drv_ip.hpp"
#include "brassert.h"
#include "video_ip.hpp"

void VIDEOI_BuildFontAtlas(HVIDEO hVideo, HVIDEO_FONT hFont, br_font *font, br_int_32 width, br_int_32 height)
{
	br_pixelmap *pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, width, height, nullptr, BR_PMAF_NORMAL);
	if(pm == nullptr)
	{
		hFont->glTex = hVideo->texture.checkerboard;
		log_error("VIDEO", "Error generating atlas for font %hux%hu.", font->glyph_x, font->glyph_y);
		return;
	}
	pm->origin_x = pm->origin_y = 0;
	BrPixelmapFill(pm, 0x00000000);

	br_rectangle r;
	r.x = 0;
	r.y = 0;
	r.w = 0;
	r.h = font->glyph_y;

	char c[2];
	c[1] = '\0';
	for(size_t i = 0; i < 256; ++i)
	{
		c[0] = (char)i;

		r.w = ((font->flags & BR_FONTF_PROPORTIONAL) ? font->width[i] : font->glyph_x);
		if((r.x + r.w) > pm->width)
		{
			--i;
			r.x = 0;
			r.y += r.h;
			ASSERT(r.y <= pm->height);
			continue;
		}

		BrPixelmapText(pm, r.x, r.y, 0xFFFFFFFF, font, c);

		/*
		** Calculate the UV coordinates.
		** * Remember that the image will be upside down when converted to a
		**     OpenGL texture. The UV for each glyph will match this.
		** * The origin of the atlas is being set to top-left@(0,0), so
		**     just swap y0/y1.
		**/
		VIDEOI_BrRectToUVs(pm, &r,
						   hFont->glyph[i].x0,
						   hFont->glyph[i].y1,
						   hFont->glyph[i].x1,
						   hFont->glyph[i].y0
		);

		r.x += r.w;
	}

	VIDEOI_BrPixelmapToExistingTexture(hFont->glTex, pm);

	BrPixelmapFree(pm);
	log_trace("VIDEO", "Successfully built atlas for font %hux%hu.", font->glyph_x, font->glyph_y);
}
