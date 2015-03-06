// C Source File
// Created 3/5/2015; 8:39:09 AM

#include "geo.h"
#include "math.h"

#include <tigcclib.h>

// Clips a polygon against a plane. Returns whether a valid polygon remains.
// TODO: keep track of which edges have been clipped so we know which
// part of the polygon still needs to be drawn
char clip_polygon_to_plane(Polygon* poly, Plane* plane, Polygon* dest) {
	short i;
	short next_point;
	short in, next_in;
	short dot, next_dot;
	short t;
	
	dot = dot_product(&poly->v[0], &plane->normal);
	in = dot >= plane->d;
	
	for(i = 0; i < poly->total_v; i++) {
		next_point = (i + 1) % poly->total_v;
		
		// The vertex is inside the plane, so don't clip it
		if(in)
			dest->v[dest->total_v++] = poly->v[i];
			
		next_dot = dot_product(&poly->v[next_point], &plane->normal);
		next_in = dot >= plane->d;
		
		// The points are on opposite sides of the plane, so clip it
		if(in != next_in) {
			// Scale factor to get the point on the plane
			t = FIXDIV8(plane->d - dot, next_dot - dot);
			
			dest->v[dest->total_v].x = FIXMUL8(poly->v[i].x + (poly->v[next_point].x - poly->v[i].x), t);
			dest->v[dest->total_v].y = FIXMUL8(poly->v[i].y + (poly->v[next_point].y - poly->v[i].y), t);
			dest->v[dest->total_v].z = FIXMUL8(poly->v[i].z + (poly->v[next_point].z - poly->v[i].z), t);
			
			dest->total_v++;
		}
		
		dot = next_dot;
		in = next_in;
	}
	
	return dest->total_v > 2;	
}

// Clips a polygon against the entire view frustum
// This routine requires two temporary polygons, one of which the
// final polygon will be in. This returns the address of which one it
// is
Polygon* clip_polygon_to_frustum(Frustum* f, Polygon* p, Polygon* temp_a, Polygon* temp_b) {
	int i;
	
	for(i = 0; i < f->total_p; i++) {
		
	}
}





#if 0
// Clips a polygon against a plane
int ClipToPlane(polygon_t *pin, plane_t *pplane, polygon_t *pout)
{
    int     i, j, nextvert, curin, nextin;
    double  curdot, nextdot, scale;
    point_t *pinvert, *poutvert;

    pinvert = pin->verts;
    poutvert = pout->verts;

    curdot = DotProduct(pinvert, &pplane->normal);
    curin = (curdot >= pplane->distance);

    for (i=0 ; i<pin->numverts ; i++)
    {
        nextvert = (i + 1) % pin->numverts;

        // Keep the current vertex if it’s inside the plane
        if (curin)
            *poutvert++ = *pinvert;

        nextdot = DotProduct(&pin->verts[nextvert], &pplane->normal);
        nextin = (nextdot >= pplane->distance);

        // Add a clipped vertex if one end of the current edge is
        // inside the plane and the other is outside
        if (curin != nextin)
        {
            scale = (pplane->distance - curdot) /
                    (nextdot - curdot);
            for (j=0 ; j<3 ; j++)
            {
                poutvert->v[j] = pinvert->v[j] +
                    ((pin->verts[nextvert].v[j] - pinvert->v[j]) *
                     scale);
            }
            poutvert++;
        }

        curdot = nextdot;
        curin = nextin;
        pinvert++;
    }

    pout->numverts = poutvert - pout->verts;
    if (pout->numverts < 3)
        return 0;

    pout->color = pin->color;
    return 1;
}

#endif