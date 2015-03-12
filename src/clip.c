// C Source File
// Created 3/5/2015; 8:39:09 AM

#include "geo.h"
#include "math.h"
#include "console.h"
#include "error.h"

#include <tigcclib.h>

// Clips a polygon against a plane. Returns whether a valid polygon remains.
// TODO: keep track of which edges have been clipped so we know which
// part of the polygon still needs to be drawn
char clip_polygon_to_plane(Polygon* poly, Plane* plane, Polygon* dest) {
	short i;
	short next_point;
	short in, next_in;
	short dot, next_dot;
	long t;
	
	dot = dot_product(&poly->v[0], &plane->normal);
	in = (dot >= plane->d);
	
	
	//printf("Dot: %d\nD: %d\n", dot, plane->d);
	//print_vex3d(&plane->normal);
	//print_vex3d(&poly->v[0]);
	//ngetchx();
	
	short total_outside = !in;
	
	dest->total_v = 0;
	
	for(i = 0; i < poly->total_v; i++) {
		next_point = (i + 1) % poly->total_v;
		
		// The vertex is inside the plane, so don't clip it
		if(in)
			dest->v[dest->total_v++] = poly->v[i];
			
		//errorif(!in, "Point not in!");
			
			
		next_dot = dot_product(&poly->v[next_point], &plane->normal);
		next_in = (next_dot >= plane->d);
		
		// The points are on opposite sides of the plane, so clip it
		if(in != next_in) {
			// Scale factor to get the point on the plane
			errorif(next_dot - dot == 0, "Clip div by 0");
			
			t = FIXDIV8(plane->d - dot, next_dot - dot);
			
			errorif(abs(t) > 32767, "Invalid clip t");
			//errorif(t == 0, "t == 0");
			
			//printf("Dist: %d\n", dot + plane->d);
			//printf("T: %d Z: %d\n", t, poly->v[i].z);
			
			dest->v[dest->total_v].x = poly->v[i].x + FIXMUL8((poly->v[next_point].x - poly->v[i].x), t);
			dest->v[dest->total_v].y = poly->v[i].y + FIXMUL8((poly->v[next_point].y - poly->v[i].y), t);
			dest->v[dest->total_v].z = poly->v[i].z + FIXMUL8((poly->v[next_point].z - poly->v[i].z), t);
			
			//printf("Dest z: %d\n", dest->v[dest->total_v].z);
			//printf("Should be: %d\n", -plane->d);
			
			//errorif(dest->v[dest->total_v].z < DIST_TO_NEAR_PLANE / 2, "Invalid clip: %d", dest->v[dest->total_v].z);
			
			++total_outside;
			
			dest->total_v++;
		}
		
		dot = next_dot;
		in = next_in;
	}
	
	//printf("total outside: %d\n", total_outside);
	
	return dest->total_v > 2;	
}

// Clips a polygon against the entire view frustum
// This routine requires two temporary polygons, one of which the
// final polygon will be in. This returns the address of which one it
// is
char clip_polygon_to_frustum(Polygon* src, Frustum* f, Polygon* dest) {
#if 1
	Polygon temp[2];
	int current_temp = 0;
	Polygon* poly = src;
	int i;
	
#if 1
	for(i = 0; i < f->total_p - 1; i++) {
		//errorif(poly->total_v < 3, "Invalid clip poly");
		if(!clip_polygon_to_plane(poly, &f->p[i], &temp[current_temp])) {
			return 0;
		}
		
		poly = &temp[current_temp];
		current_temp = !current_temp;
	}
#endif
	
	//return poly->total_v > 2;
	return clip_polygon_to_plane(poly, &f->p[f->total_p - 1], dest);
	
#endif
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