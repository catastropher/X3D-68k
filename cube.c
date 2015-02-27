// C Source File
// Created 2/16/2015; 11:24:28 PM

#include <tigcclib.h>

#include "3D.h"
#include "extgraph.h"
#include "graphics.h"

const int cube_vertex_tab[6][5] = {
	{2, 1, 0, 3, 2},
	{6, 7, 4, 5, 6},
	{0, 1, 5, 4, 0},
	{2, 3, 7, 6, 2},
	{2, 6, 5, 1, 2},
	{3, 0, 4, 7, 3}
};

typedef struct {
	Vex3D v[8];
	Vex3D normal[8];
	char color[8];
	short cube[6];
	
	unsigned short last_frame;
	unsigned short edge_bits;
} Cube;

typedef struct {
	Vex3D v[4];
} Face;

#if 1
typedef struct{
	short x;
	short pad1;
	short y;
	short pad2;
	short z;
	short pad3;
} Vex3D_rot;
#else
typedef struct{
	short pad1;
	short x;
	short pad2;
	short y;
	short pad3;
	short z;
} Vex3D_rot;
#endif

short rotate_point_local(Vex3D_rot *dest asm("a3"), Vex3D *src asm("a4"), Mat3x3* mat asm("a5"));

#if 0
void cube_get_face(Face* dest, short x[], short y[], int face) {
	int i;
	
	for(i = 0; i < 4; i++)
		dest->v[i] = (Vex2D){x[cube_vertex_tab[face][i]], y[cube_vertex_tab[face][i]]};
}
#endif

inline void cube_get_face_3D(Face* dest, Vex3D_rot v[], int face) {
	int i;
	
	for(i = 0; i < 4; i++)
		dest->v[i] = (Vex3D){v[cube_vertex_tab[face][i]].x, v[cube_vertex_tab[face][i]].y, v[cube_vertex_tab[face][i]].z};
		//(Vex2D){x[cube_vertex_tab[face][i]], y[cube_vertex_tab[face][i]]};
}

extern void* Vscreen0;
extern void* Vscreen1;

inline long dot_product(Vex3D* a, Vex3D* b) {
	return (((long)a->x * b->x) >> 2) + (((long)a->y * b->y) >> 2) + (((long)a->z * b->z) >> 2);
}

short scale_factor;
extern short lcd_w, lcd_h;

inline char polygon_visible(Vex3D* normal, Vex3D* cam_pos, Vex3D* v) {
	Vex3D diff = {v->x - cam_pos->x, v->y - cam_pos->y, v->z - cam_pos->z};
	
	long dot = (long)normal->x * diff.x + (long)normal->y * diff.y + (long)normal->z * diff.z;
	
	return dot < 0;
	
	
	
	//long dot = (long)view->x * normal->x + (long)view->y * normal->y + (long)view->z * normal->z;
	
	//printf("Dot: %ld\n", dot);
	
	#if 0
	char buf[32];
	
	if(outline) {
		PortSet(Vscreen0, 239, 127);
		sprintf(buf, "%ld %d %d %d", dot, normal->x, normal->y, normal->z);
		
		DrawStr(0, 15 + id * 15, buf, A_NORMAL);
	}
	
	if(dot == 0) {
		
		long d = (long)point->x * normal->x + (long)point->y * normal->y + (long)point->z * normal->z;
		
		d >>= 2;
		
		//printf("Dot is 0\n");
		
		//error("DOT is 0");
		
		long res = (((long)normal->x * cam_pos->x) >> 2) +
			(((long)normal->y * cam_pos->y) >> 2) +
			(((long)normal->x * cam_pos->x) >> 2) -
			d;
			
		return res > 0;
			
	}
	
	// Was 6
	
	return dot < -21474836;
	//return dot < 0;//dot < -1000000 * 3;
	#endif
}


#define MIN 10

int was_clipped;

char clip_ray(Vex3D* v1, Vex3D* v2) {
	was_clipped = 0;
	if(v1->z < MIN && v2->z < MIN) return 0;

	if(v1->z >= MIN && v2->z >= MIN) return 1;

	//cout << "Need clip" << endl;

	if(v1->z < MIN) {
		short dx = v1->x - v2->x;
		short dy = v1->y - v2->y;
		short dz = v1->z - v2->z;
		
		short t = (short)((((long)(MIN - v1->z)) << 15) / dz);
		
		v1->x = (((long)dx * t) >> 15) + v1->x;
		v1->y = (((long)dy * t) >> 15) + v1->y;
		v1->z = (((long)dz * t) >> 15) + v1->z;
		was_clipped = 0;
	}
	else {
		short dx = v2->x - v1->x;
		short dy = v2->y - v1->y;
		short dz = v2->z - v1->z;
		
		short t = (short)((((long)(MIN - v2->z)) << 15) / dz);
		
		v2->x = (((long)dx * t) >> 15) + v2->x;
		v2->y = (((long)dy * t) >> 15) + v2->y;
		v2->z = (((long)dz * t) >> 15) + v2->z;
		was_clipped = 1;
	}

#if 0
	
		double dz = v1->z - v2->z;
		t = ((double)MIN - v1->z) / dz;

		v1->x = ((double)v1->x - v2->x) * t + v1->x;
		v1->y = ((double)v1->y - v2->y) * t + v1->y;
		v1->z = ((double)v1->z - v2->z) * t + v1->z;
		was_clipped = 0;
	}
	else {
		double dz = v2->z - v1->z;
		t = ((double)MIN - v2->z) / dz;

		v2->x = ((double)v2->x - v1->x) * t + v2->x;
		v2->y = ((double)v2->y - v1->y) * t + v2->y;
		v2->z = ((double)v2->z - v1->z) * t + v2->z;
		was_clipped = 1;
	}

	//if(!(v1.z >= MIN*.90 && v2.z >= MIN*.90)){
	//	cout << "Error!" << endl;
	//}
#endif

	return 1;
}


/*
typedef struct {
	Vex2D v[3];
} Triangle2D;

short 

void triangle_clip_2D_top(Triangle* t, Triangle2D dest[], short *total_tri) {
	if(v[2].y < 0) {
		*total_tri = 0;
	}
	else if(v[1].y < 0) {
		
	}
	
	
	

	if(v[0].y >= 0) {
		dest[0] = *t;
		*total_tri = 1;
	}
	
	
	
}
*/

void XGrayFilledTriangle_R(short x1 asm("%d0"),short y1 asm("%d1"),short x2 asm("%d2"),short y2 asm("%d3"),short x3 asm("%d4"),short y3 asm("%a1"),void *planes asm("%a0"), void(*drawfunc)(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0")) asm("%a2"));

inline void line_info(Line* dest, Vex2D* start, Vex2D* end) {
	short dx = end->x - start->x;
	short dy = end->y - start->y;
	
	dest->slope = ((long)dx << LINE_BITS) / dy;
	dest->start = *start;
}

short line_intersect(Line* line, short y) {
	return line->start.x + (((long)line->slope * (y - line->start.y)) >> LINE_BITS);
}

//void(*drawfunc)(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0")

void Xasm_gray_tri(CBuffer* buf, short x1, short y1, short x2, short y2, short x3, short y3, short color) {
	void* call_back[] = {
		GrayDrawSpan_BLACK_R,
		GrayDrawSpan_LGRAY_R,
		GrayDrawSpan_DGRAY_R,
		GrayDrawSpan_WHITE_R
	};
	
	//rasterize_triangle_full(buf, x1, y1, x2, y2, x3, y3, color);
	
	
	XGrayFilledTriangle_R(x1, y1, x2, y2, x3, y3, buf->dark_plane, DrawSpan_REVERSE_R);
	
	//draw_clip_tri(x1, y1, x2, y2, x3, y3, buf->dark_plane);
	//XGrayFilledTriangle_R(x1, y1, x2, y2, x3, y3, Vscreen1, call_back[color]);
}


void project_draw_tri(CBuffer* buf, Vex3D *v, short color, char draw_01, char draw_02, char draw_12){
	Vex2D v2[3];
	Vex2D out;
	int i;
	
	//if(buf->lines_left == 0)
	//	return;
	
	short width = buf->width;
	short cx = width / 2;
	short cy = buf->height / 2;

	for(i = 0;i < 3;i++){
		//rot_out = v[i];
		//project_vex3(&out);
		//v2[i].x = out.x;
		//v2[i].y = out.y;
		//v2[i].z = 0;
		
		v2[i].x = (((long)buf->scale_factor * v[i].x) / (v[i].z)) + cx;
		v2[i].y = (((long)buf->scale_factor * v[i].y) / (v[i].z)) + cy;
	}
	
#if 0
	v2[0] = (Vex2D){10, -10};
	v2[1] = (Vex2D){10, 100};
	v2[2] = (Vex2D){100, 100};
#endif

	//render_triangle(v2, 0, id);
	
#ifdef MODE_GRAY
	Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
#endif
	
	//asm_gray_tri(v2[0].x, v2[0].y, v2[1].x, v2[1].y, v2[2].x, v2[2].y, color);
	
	char changed;
	
#if 0
	if(v2[0].x < 0 && v2[1].x < 0 && v2[2].x < 0)
		return;
	
	if(v2[0].x > width && v2[1].x > width && v2[2].x > width)
		return;
#endif

	short o[] = {0, 1, 2};
	
	do {
		changed = 0;
		for(i = 0; i < 2; i++)
			if(v2[i].y > v2[i + 1].y) {
				changed = 1;
				Vex2D temp = v2[i];
				v2[i] = v2[i + 1];
				v2[i + 1] = temp;
			}
	} while(changed);
	
	char tab[3][3];
	
#if 0
	tab[o[0]][o[1]] = draw_01;
	tab[o[1]][o[0]] = draw_01;
	tab[o[0]][o[2]] = draw_02;
	tab[o[2]][o[0]] = draw_02;
	tab[o[1]][o[2]] = draw_12;
	tab[o[2]][o[1]] = draw_12;
	
	
	Vex2D v2[3];
	
	for(i = 0; i < 3; i++)
		v2[i] = vx[o[i]];
#endif
	
	
	if(v2[2].y < 0)
		return;
	else if(v2[1].y < 0) {
		// Two of the points are above the screen
		Line a, b;
		
		line_info(&a, &v2[0], &v2[2]);
		line_info(&b, &v2[1], &v2[2]);
		
		#if 1
		v2[0].y = 1;
		v2[0].x = line_intersect(&a, 0);
		
		v2[1].y = 1;
		v2[1].x = line_intersect(&b, 0);
		#endif
		
		//buf->draw[0] = 
		
		Xasm_gray_tri(buf, v2[0].x, v2[0].y, v2[1].x, v2[1].y, v2[2].x, v2[2].y, color);
		
	}
	else if(v2[0].y < 0) {
		// One of the points is above the screen
		Line a, b;
		
		line_info(&a, &v2[0], &v2[2]);
		line_info(&b, &v2[0], &v2[1]);
		
		Vex2D new_pa, new_pb;
		
		new_pa.y = 0;
		new_pa.x = line_intersect(&a, 0);
		
		new_pb.y = 0;
		new_pb.x = line_intersect(&b, 0);
		
		//asm_gray_tri(v2[0].x, v2[0].y, v2[1].x, v2[1].y, v2[2].x, v2[2].y, color);
		//return;
		// Ugh, we have no choice but to draw two triangles (the result of clipping gives
		// a quadrilateral)
		
		//char rasterize_triangle_half(short slope_left, short slope_right, short y, short end_y, short* x_left, short* x_right, short color)
		
	#ifdef MODE_GRAY
		if(!rasterize_triangle_half(buf, a.slope, b.slope, 0, v2[1].y, &new_pa.x, &new_pb.x, color)) {
			Line c;
			line_info(&c, &v2[1], &v2[2]);
			
			if(v2[1].y != v2[2].y)
				rasterize_triangle_half(buf, a.slope, c.slope, v2[1].y, v2[2].y, &new_pa.x, &new_pb.x, color);
		}
	#else
		
		
		//error("Above");
		
		Xasm_gray_tri(buf, v2[2].x, v2[2].y, new_pa.x, new_pa.y, v2[1].x, v2[1].y, color);
		
		//Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
		//Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
		Xasm_gray_tri(buf, new_pb.x, new_pb.y, new_pa.x, new_pa.y, v2[1].x, v2[1].y, color);
		
	#endif
		
	}
	else if(v2[0].y < buf->height) {
		Xasm_gray_tri(buf, v2[0].x, v2[0].y, v2[1].x, v2[1].y, v2[2].x, v2[2].y, color);
	}
		
		
		
		
		
		
		
		
		
		
		
	//if(v2[0].y > -10) {
		//asm_gray_tri(v2[0].x, v2[0].y, v2[1].x, v2[1].y, v2[2].x, v2[2].y, color);
	//}
	//else {
		
	//}
	
	//draw_clip_tri(v2[0].x, v2[0].y, v2[1].x, v2[1].y, v2[2].x, v2[2].y, Vscreen0);
}

void clip_triangle(CBuffer* buf, Vex3D* v, short color, char draw_a, char draw_b) {
	int out[3];
	int in[3];
	int pos_o = 0;
	int pos_i = 0;
	int i;
	
	//if(buf->lines_left == 0)
	//	return;

	for(i = 0;i < 3;i++){
		if(v[i].z < MIN)	out[pos_o++] = i;
		else				in[pos_i++] = i;
	}

	if(pos_i == 3) {
		//cout << "Case1" << endl;
		project_draw_tri(buf, v, color, 0, 0, 0);
	}
	else if(pos_i == 2) {
		//cout << "Case2" << endl;
		Vex3D o1 = v[out[0]];
		Vex3D o2 = v[out[0]];

		clip_ray(&o1,&v[in[0]]);
		clip_ray(&o2,&v[in[1]]);

		Vex3D list1[3] = {o1,v[in[0]],v[in[1]]};
		Vex3D list2[3] = {o1,o2,v[in[1]]};

		
		/*
		 
            /\		 
		 ----------
		  /    \
		 --------
 		 
		 
		*/
		
		
		project_draw_tri(buf, list1, color, 0, 0, 0);
		project_draw_tri(buf, list2, color, 0, 0 ,0);
	}
	else if(pos_i == 1) {
		//cout << "Case3" << endl;
		Vex3D o1 = v[out[0]];
		Vex3D o2 = v[out[1]];

		clip_ray(&o1,&v[in[0]]);
		clip_ray(&o2,&v[in[0]]);

		Vex3D list1[3] = {o1,o2,v[in[0]]};

		project_draw_tri(buf, list1, color, 0, 0, 0);
	}
	else{
		//cout << "INVISIBLE" << endl;
		//print_vex3(&v[0]);
		//print_vex3(&v[1]);
		//print_vex3(&v[2]);
	}
}

inline short get_color(Vex3D* cam_dir, Vex3D* normal) {
	//Vex3D reverse = {-normal->x, -normal->y, -normal->z};
	
	long dot = dot_product(cam_dir, normal);
	long a = 90177536;
	
	if(dot < -3 * a + a / 4 - a / 8)
		return COLOR_WHITE;
	else if(dot < -a)
		return COLOR_LIGHTGRAY;
	
	return COLOR_DARKGRAY;
	
}

inline void project_3D(CBuffer* buf, Vex3D* src, Vex2D* dest) {
	short cx = buf->width / 2;
	short cy = buf->height / 2;
	
	dest->x = (((long)buf->scale_factor * src->x) / (src->z)) + cx;
	dest->y = (((long)buf->scale_factor * src->y) / (src->z)) + cy;
}

extern long seg_calls;

void render_ray(CBuffer* buf, Vex3D* v1,Vex3D *v2) {
	Vex3D a = *v1, b = *v2;
	
	seg_calls++;
	
	if(!clip_ray(&a, &b)) return;

	//print_vex3(&v1);
	//print_vex3(&v2);

	Vex2D s1,s2;

	//rot_out = v1;
	//project_vex3(&s1);
	project_3D(buf, &a, &s1);
	project_3D(buf, &b, &s2);

	//rot_out = v2;
	//project_vex3(&s2);
	
	//if(s1.x < 0 || s1.y < 0 || s1.x > 239 || s1.y > 127) return;
	//if(s2.x < 0 || s2.y < 0 || s2.x > 239 || s2.y > 127) return;
	
	short max_x = buf->width - 1;
	short max_y = buf->height - 1;

	if(s1.x < 0 && s2.x < 0) return;
	if(s1.x > max_x && s2.x > max_x) return;

	if(s1.y < 0 && s2.y < 0) return;
	if(s1.y > max_y && s2.y > max_y) return;

	//GrayFastDrawLine2B(s1.x, s1.y, s2.x, s2.y, COLOR_LIGHTGRAY, Vscreen0, Vscreen1);
	
	if(s1.x < 0 || s1.y < 0 || s1.x > max_x || s1.y > max_y || s2.x < 0 || s2.y < 0 || s2.x > max_x || s2.y > max_y) {
		//draw_clip_line(s1.x, s1.y, s2.x, s2.y, Vscreen1);
		draw_clip_line(s1.x, s1.y, s2.x, s2.y, buf->dark_plane);
	}
	else {
		FastLine_Draw_R(buf->dark_plane, s1.x, s1.y, s2.x, s2.y);
		//FastDrawLine(buf->dark_plane, s1.x, s1.y, s2.x, s2.y, A_NORMAL);
		//GrayFastDrawLine2B_R(Vscreen0, Vscreen1, s1.x, s1.y, s2.x, s2.y, COLOR_BLACK);
	}
}

#define TOTAL_CUBES 5

Cube cube_tab[TOTAL_CUBES];

// Given a vertex id a and id b, edge_table[a][b] gets the edge id between a and b
char edge_table[8][8];
short edge_vertex_table[12][2];

// Given the face id, this gets the bitset of the edges in that face
unsigned short edge_face_table[6];

short get_opposite_face(short face) {
	if(face & 1)
		return face - 1;
	else
		return face + 1;
}

void cube_pass_edges(CBuffer* buf, Cube* to, short face) {
	if(buf->frame != to->last_frame) {
		to->edge_bits = 0;
		to->last_frame = buf->frame;
	}
	
	
	to->edge_bits |= edge_face_table[get_opposite_face(face)];
}

void build_edge_table() {
	int i, d;
	
	short id = 0;
	
	for(i = 0; i < 8; i++)
		for(d = 0; d < 8; d++)
			edge_table[i][d] = -1;
	
	// There's probably a better way to do this... oh well
	for(i = 0; i < 6; i++) {
		unsigned short bit = 0;
		
		for(d = 0; d < 4; d++) {
			short a = cube_vertex_tab[i][d];
			short b = cube_vertex_tab[i][d + 1];
			
			if(edge_table[a][b] == -1) {
				bit |= ((unsigned short)1 << id);
				
				edge_vertex_table[id][0] = a;
				edge_vertex_table[id][1] = b;
				
				edge_table[a][b] = id;
				edge_table[b][a] = id++;
			}
			else {
				bit |= ((unsigned short)1 << edge_table[a][b]);
			}
		}
		
		edge_face_table[i] = bit;
	}
	
	for(i = 0; i < 6; i++) {
		unsigned short b = edge_face_table[i];
		
		for(d = 0; d < 12; d++) {
			if(b & (1 << 11)) {
				printf("1");
			}
			else {
				printf("0");
			}
			
			b <<= 1;
		}
		printf("\n");
	}
	
	ngetchx();
	
	for(i = 0; i < 8; i++) {
		for(d = 0; d < 8; d++) {
			printf("%d ", edge_table[i][d]);
		}
		printf("\n");
	}
	
	if(id != 12)
		error("ERR");
}


void render_cube(CBuffer* buf, Cube* c, Cam* cam, char outline) {
	Mat3x3* mat = &cam->mat;
	Vex3D* view = &cam->dir;
	Vex3D* cam_pos = &cam->pos;
	
	//if(buf->lines_left == 0)
	//	return;
		
	c->edge_bits |= (1 << 15);
	
	short cx = buf->width / 2;
	short cy = buf->height / 2;
	Vex3D_rot output[8];
	short x[8];
	short y[8];
	char vis[6];
	
	int i, d;
	
	for(i = 0; i < 8; i++) {
		Vex3D p = {c->v[i].x - cam_pos->x, c->v[i].y - cam_pos->y, c->v[i].z - cam_pos->z};
		
		rotate_point_local(&output[i], &p, mat); 
		
		x[i] = (((long)buf->scale_factor * output[i].x) / (output[i].z)) + cx;
		y[i] = (((long)buf->scale_factor * output[i].y) / (output[i].z)) + cy;
		
		//printf("x[i] = %d\n", y[i]);
	}
	
	//unsigned char edge_table[8];
	
	short ctab[4];
	
	//for(i = 0; i < 8; i++)
	//	edge_table[i] = 0;
	
	if(1) {
		for(i = 0; i < 6; i++) {
			Vex3D_rot normal;
			
			//rotate_point_local(&normal, &c->normal[i], mat);
			vis[i] = polygon_visible(&c->normal[i], cam_pos, &c->v[cube_vertex_tab[i][0]]);
			
			if(vis[i] && c->cube[i] != -1) {
				Cube* c2 = &cube_tab[c->cube[i]];
			
				cube_pass_edges(buf, c2, i);

				if(!(c2->edge_bits & (1 << 15))) {
					render_cube(buf, c2, cam, outline);
				}
			}
		}
		
		if(!buf->first_cube) {
			for(i = 0; i < 6; i++) {	
				if(vis[i]) {
					//if(i != 5)
					//	error("NOT 5");
					
					if(c->cube[i] == -1) {
						Face face;
						cube_get_face_3D(&face, output, i);
						
						Vex3D tri1[] = {face.v[0], face.v[1], face.v[2]};
						Vex3D tri2[] = {face.v[0], face.v[3], face.v[2]};
						
						short color = get_color(&cam->dir, &c->normal[i]);
						
						if(outline)
							color = c->color[i];
							
						ctab[i] = color;
						
						char draw[4];
							
						
						clip_triangle(buf, tri1, color, draw[0], draw[1]);
						
						clip_triangle(buf, tri2, color, draw[3], draw[2]);
					}
					
					
					/*
					if(outline) {
						for(d = 0; d < 3; d++){
							render_ray(&face.v[d], &face.v[d + 1]);
						}
				
						render_ray(&face.v[0], &face.v[3]);
					}*/
					
					
					
					//render_ray(list2[v1],list2[v4]);
					
					
					#if 0
					Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
					Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
					asm_gray_tri(face.v[0].x, face.v[0].y, face.v[1].x, face.v[1].y, face.v[2].x, face.v[2].y, c->color[i]);
					//printf("%d is visible\n", i);
					
					Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
					Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
					asm_gray_tri(face.v[0].x, face.v[0].y, face.v[3].x, face.v[3].y, face.v[2].x, face.v[2].y, c->color[i]);
					#endif
				
					
					//printf("Vis: %d\n", i);
					
				}
			}
		
			//GrayDBufToggleSync();
			//ngetchx();
			//GrayDBufToggleSync();
			
			
				
		}
	#ifdef MODE_GRAY
		for(i = 0; i < 6; i++) {
			if(vis[i] && c->cube[i] != -1) {
				Cube* c2 = &cube_tab[c->cube[i]];
			
				cube_pass_edges(buf, c2, i);

				if(!(c2->edge_bits & (1 << 15))) {
					render_cube(buf, c2, cam, outline);
				}
			}
		}
	#endif
	
		
	// New edge drawing code!
	unsigned short edge = c->edge_bits;
	
	for(i = 0; i < 12; i++) {
		if((edge & 1) == 0) {
			// This edge hasn't been drawn yet, so draw it
			short a = edge_vertex_table[i][0];
			short b = edge_vertex_table[i][1];
			
			Vex3D aa = {output[a].x, output[a].y, output[a].z};
			Vex3D bb = {output[b].x, output[b].y, output[b].z};
			
			render_ray(buf, &aa, &bb);
			
			if(outline) {
				LCD_restore(buf->dark_plane);
				ngetchx();
			}
		}
		
		edge >>= 1;
	}
	
	
		
	#if 0
		if(1) {
			for(i = 0; i < 6; i++) {
				if(vis[i]) {// && ctab[i] == COLOR_WHITE) {
					
					unsigned char edge_table[8];
					
					for(d = 0; d < 8; d++)
						edge_table[i] = 0;	
					
					seg_calls++;
					
					Face face;
					cube_get_face_3D(&face, output, i);	
				
					for(d = 0; d < 3; d++) {
						short a = cube_vertex_tab[i][d];
						short b = cube_vertex_tab[i][d + 1];
						
						if(b < a) {
							short temp = a;
							a = b;
							b = temp;
						}
						
						if(!(edge_table[a] & (1 << b))) {
							render_ray(buf, &face.v[d], &face.v[d + 1]);
							edge_table[a] |= (1 << b);
							
							
							
							if(outline) {
								LCD_restore(buf->dark_plane);
								ngetchx();
							}
						}
					}
					
					short a = cube_vertex_tab[i][0];
					short b = cube_vertex_tab[i][3];
					
					if(b < a) {
						short temp = a;
						a = b;
						b = temp;
					}
					
					if(!(edge_table[a] & (1 << b))) {
						render_ray(buf, &face.v[0], &face.v[3]);
						edge_table[a] |= (1 << b);
						
						
						
						if(outline) {
							LCD_restore(buf->dark_plane);
							ngetchx();
						}
					}
				}
			}
		}
		#endif
	}
	else {
		
		#if 0
		for(i = 0; i < 6; i++) {
			//polygon_visible(view, &c->normal[i], cam_pos, &output[cube_vertex_tab[i][0]], i, outline, &c->v[cube_vertex_tab[i][0]]);
		}
		
		// Top
		for(i = 0; i < 3; i++) {
			GrayDrawClipLine2B(x[i], y[i], x[i + 1], y[i + 1], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		}
		
		GrayDrawClipLine2B(x[0], y[0], x[3], y[3], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		
		// Bottom
		for(i = 4; i < 7; i++) {
			GrayDrawClipLine2B(x[i], y[i], x[i + 1], y[i + 1], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		}
		
		GrayDrawClipLine2B(x[4], y[4], x[7], y[7], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		
		// Sides
		for(i = 0; i < 4; i++) {
			GrayDrawClipLine2B(x[i], y[i], x[i + 4], y[i + 4], COLOR_DARKGRAY, Vscreen0, Vscreen1);
		}
		
		#endif
		
	}
	
	buf->first_cube = 0;
}

enum{
	VEX_UBL,
	VEX_UTL,
	VEX_UTR,
	VEX_UBR,
	VEX_DBL,
	VEX_DTL,
	VEX_DTR,
	VEX_DBR
};

enum {
	PLANE_BOTTOM,
	PLANE_TOP,
	PLANE_LEFT,
	PLANE_RIGHT,
	PLANE_BACK,
	PLANE_FRONT
};

inline Vex3D mVex3D(int x, int y, int z) {
	return (Vex3D){x, y, z};
}

void make_cube(Cube* c, int x ,int y, int z, int posx, int posy, int posz, Vex3D* angle){
	x/=2;
	y/=2;
	z/=2;

	c->v[VEX_UBL] =  mVex3D(-x, y, -z);
	c->v[VEX_UTL] =  mVex3D(-x, y, z);
	c->v[VEX_UTR] =  mVex3D(x, y, z);
	c->v[VEX_UBR] =  mVex3D(x, y, -z);
	
	c->v[VEX_DBL] =  mVex3D(-x, -y, -z);
	c->v[VEX_DTL] =  mVex3D(-x, -y, z);
	c->v[VEX_DTR] =  mVex3D(x, -y, z);
	c->v[VEX_DBR] =  mVex3D(x, -y, -z);
	
	int v = -32767;
	
	#if 1
	c->normal[PLANE_BOTTOM] = mVex3D(0, v, 0);
	c->normal[PLANE_TOP] = mVex3D(0, -v, 0);
	c->normal[PLANE_FRONT] = mVex3D(0, 0, -v);
	c->normal[PLANE_BACK] = mVex3D(0, 0, v);
	c->normal[PLANE_LEFT] = mVex3D(-v, 0, 0);
	c->normal[PLANE_RIGHT] = mVex3D(v, 0, 0);
	
	c->color[PLANE_BOTTOM] = COLOR_BLACK;
	c->color[PLANE_TOP] = COLOR_BLACK;
	c->color[PLANE_LEFT] = COLOR_LIGHTGRAY;
	c->color[PLANE_RIGHT] = COLOR_LIGHTGRAY;
	c->color[PLANE_FRONT] = COLOR_DARKGRAY;
	c->color[PLANE_BACK] = COLOR_DARKGRAY;
	#endif
	
	c->normal[PLANE_BOTTOM] = mVex3D(0, v, 0);
	c->normal[PLANE_TOP] = mVex3D(0, -v, 0);
	c->normal[PLANE_FRONT] = mVex3D(0, 0, -v);
	c->normal[PLANE_BACK] = mVex3D(0, 0, v);
	c->normal[PLANE_LEFT] = mVex3D(-v, 0, 0);
	c->normal[PLANE_RIGHT] = mVex3D(v, 0, 0);
	
	
	int i;
	
	for(i = 0; i < 6; i++)
		c->cube[i] = -1;
	
#if 0
	Mat3x3 mat, mat2;
	x3d_construct_mat3_old(angle, &mat);
	
	x3d_construct_mat3(angle, &mat2);
	
	for(i = 0; i < 8; i++) {
		Vex3D_rot rot;
		rotate_point_local(&rot, &c->v[i], &mat);
		c->v[i].x = rot.x;
		c->v[i].y = rot.y;
		c->v[i].z = rot.z;
	}
	
	for(i = 0; i < 6; i++) {
		Vex3D_rot rot;
		rotate_point_local(&rot, &c->normal[i], &mat);
		c->normal[i].x = rot.x;
		c->normal[i].y = rot.y;
		c->normal[i].z = rot.z;
	}
#endif
	
	for(i = 0; i < 8; i++) {
		c->v[i].x += posx;
		c->v[i].y += posy;
		c->v[i].z += posz;
	}
}


void init_cubes() {
	Vex3D cube_angle = {0, 0, 0};
	
	make_cube(&cube_tab[0], 75, 75, 75, 0, 0, 0, &cube_angle);
	make_cube(&cube_tab[1], 75, 75, 75, 0, 0, 75, &cube_angle);
	make_cube(&cube_tab[2], 75, 75, 75, -75, 0, 75, &cube_angle);
	
	cube_tab[0].cube[PLANE_BACK] = 1;
	cube_tab[1].cube[PLANE_LEFT] = 2;
}

void invert_normals() {
	int i, d;
	
	for(i = 0; i < TOTAL_CUBES; i++) {
		for(d = 0; d < 6; d++) {
			cube_tab[i].normal[d].x = -cube_tab[i].normal[d].x;
			cube_tab[i].normal[d].y = -cube_tab[i].normal[d].y;
			cube_tab[i].normal[d].z = -cube_tab[i].normal[d].z;
		}
	}
}

void test_cube(CBuffer* buf, Cam* cam, char outline, Vex3D* cube_angle) {
	Cube c, c2;
	
	buf->first_cube = 1;
	cube_tab[0].last_frame = buf->frame;
	cube_tab[0].edge_bits = 0;
	
	render_cube(buf, &cube_tab[0], cam, outline);
	return;
	
	
	
	Vex3D cube_angle2 = {cube_angle->y, -cube_angle->y + 360, 0};
	
	if(cube_angle2.y >= 360)
		cube_angle2.y -= 360;
	
	make_cube(&c, 75, 75, 75, 0, 0, 0, cube_angle);
	//make_cube(&c2, 40, 40, 40, 0, -70, 200, &cube_angle2);
	
	Vex3D angle = {0, 0, 0};
	
	
	//cam->dir = cam->mat[0][1];
	
	int i, j;
	
	/*
	for(j = 0; j < 2; j++) {
		for(i = 0; i < 5; i++) {
			render_cube(&c, &mat, &view, &pos, 30 + i * 40, 64 - 25 + 45 * j, outline);
		}
	}
	*/
	
	Cube c3;
	
	//make_cube(&c3, 20, 20, 20, 0, 0, -200, cube_angle);
	
	//for(i = 0; i < 8; i++) {
	//	c3.v[i].z -= 200;
	//}
	
	render_cube(buf, &c, cam, outline);
	//render_cube(&c2, cam, outline);
	//render_cube(&c3, cam, outline);
	
	//render_cube_outline(&c, &mat, &view, &pos, 120 + 40, 64, outline);
}


short get_int(const char *prompt) {
	char buf[32];
	
	printf("%s: ", prompt);
	gets(buf);
	printf("\n");
	
	return atoi(buf);
}

void test_vector() {
	return;
	Vex3D v;
	Vex3D angle;
	Vex3D_rot out;
	
	clrscr();
	
	do {
		printf("==========\n");
		v.x = get_int("v.x");
		v.y = get_int("v.y");
		v.z = get_int("v.z");
		
		angle.x = get_int("angle.x");
		angle.y = get_int("angle.y");
		angle.z = get_int("angle.z");
		
		Mat3x3 mat;
		
		x3d_construct_mat3(&angle, &mat);
		
		
		rotate_point_local(&out, &v, &mat);
		
		printf("\n{%d, %d, %d}\n", out.x, out.y, out.z);
	} while(1);
	
	
}

void test_rotate() {
	Mat3x3 mat;
	Vex3D v = {40, 0, 0};
	Vex3D angle = {0, 0, 0};
	Vex3D_rot out;
	
	do {
		x3d_construct_mat3(&angle, &mat);
		rotate_point_local(&out, &v, &mat);
		//clrscr();
		DrawLine(120, 64, 120 + out.x, 64 + out.z, A_NORMAL);
		
		angle.y = (angle.y + 1) % 360;
	} while(1);
}












































