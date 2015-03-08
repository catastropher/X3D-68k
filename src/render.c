// C Source File
// Created 3/7/2015; 6:28:25 PM

#include "geo.h"

#include <tigcclib.h>

#if 0
typedef struct RenderContext {
	short w, h;
	short x, y;
	short scale;
	
	Frustum frustum;
} RenderContext;
#endif


// Initializes a render context
void init_render_context(short w, short h, short x, short y, unsigned char fov, RenderContext* c) {
	c->w = w;
	c->h = h;
	c->x = x;
	c->y = y;
	c->fov = fov;

	// Calulate the distance to the project plane (which is the scale)
	// dist = (w / 2) / tan(fov / 2)
	c->scale = FIXDIV8(w / 2, tanfp(fov / 2));
	
	
}