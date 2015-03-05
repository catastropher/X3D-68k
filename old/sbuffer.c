// C Source File
// Created 2/21/2015; 1:00:22 AM

#include <tigcclib.h>

#include "extgraph.h"
#include "sbuffer.h"
#include "mem.h"

extern void *Vscreen0, *Vscreen1;


//Seg **head_ptr;
//Seg *seg_pool;
//short total_segs;
//short seg_pos;

void render_segs() {
#if 0
	int i;
	
	for(i = 0;i < 128; i++) {
		Seg* s = head_ptr[i];
		
		while(s) {
			PortSet(Vscreen1, 239, 127);
			DrawPix(s->left, i, A_NORMAL);
			DrawPix(s->right, i, A_NORMAL);
			PortSet(Vscreen0, 239, 127);
			DrawLine(s->left, i, s->right, i, A_NORMAL);
			s = s->next;
		}
	}
#endif
}

Seg *alloc_seg(CBuffer* buf) {
	buf->seg_pool[buf->seg_pos].next = NULL;
	return &buf->seg_pool[buf->seg_pos++];
}

void init_cbuffer(CBuffer* buf, short segs) {
	buf->seg_pool = alloc_mem(sizeof(Seg) * segs);
	buf->seg_pos = 0;
	buf->total_segs = segs;
}

extern short lcd_h;

void reset_cbuffer(CBuffer* buf) {
#if 0	
	int i;
	short height = buf->height;
	
	for(i = 0;i < height;i++){
		head_ptr[i] = NULL;
	}
	
	seg_pos = 0;
#endif

	memset(buf->lines, 0, sizeof(Scanline) * MAX_HEIGHT);
	buf->seg_pos = 0;
	buf->lines_left = buf->height;
	
}

void validate_segs() {
#if 0
	int i;
	int count;
	
	for(i = 0; i < 128; i++) {
		Seg* s = head_ptr[i];
		count = 0;
		
		while(s) {
			count++;
			
			if(s->left > s->right || (s->next && (s->next->left <= s->right)) || count > 10) {
				GrayOff();
				PortSet(LCD_MEM, 239, 127);
				clrscr();
				printf("Segs out of order\n");
				
				s = head_ptr[i];
				
				while(s) {
					printf("{%d, %d} -> %d\n", s->left, s->right, s->c);
					s = s->next;
				}
				
				ngetchx();
				exit(-1);
				
			}
			
			s = s->next;
		}
	}
#endif
}

void print_segs() {
#if 0
	Seg* s;
	s = head_ptr[70];
		
	GrayOff();		
	while(s) {
		printf("{%d, %d} -> %d\n", s->left, s->right, s->c);
		s = s->next;
	}
	
	ngetchx();
	exit(-1);
#endif
}

extern short lcd_w, lcd_h;

short signof(short x) {
	if(x == 0) return 0;
	return (x < 0 ? -1 : 1);
}

inline void draw_seg(CBuffer* buf, short y, short left, short right, short color) {
	//int i;
	
	//short *screen = (short *)SCREEN_BASE_ADDRESS + y*320 + left;
	
	//for(i = left;i<=right;i++)
	//drawLine_(left,y,right,y,color);
	
	if(y < 0 || y >= buf->height)
		error("Invalid y");
	
	void* addr = buf->dark_plane + (240 / 8) * y;
	
	short x = left;
	short normal_length = right - left;
	
	/*if(left - buf->pix_left_start <= buf->pix_left_dx) {
		GrayDrawSpan_BLACK_R(left, left - buf->pix_left_dx + signof(buf->pix_left_dx), addr);
		left += (left - buf->pix_left_dx) - left;
	}
	
	if(right - buf->pix_right_start <= buf->pix_right_dx) {
		GrayDrawSpan_BLACK_R(right, right - buf->pix_right_dx + signof(buf->pix_right_dx), addr);
		right += (right - buf->pix_right_dx) - right;
	}
	
	return;
	
	if(left > right)
		return;
	
	*/
	#if 1
	switch(color) {
		case COLOR_BLACK:
			GrayDrawSpan_BLACK_R(left, right, addr);
			break;
		case COLOR_LIGHTGRAY:
			FastDrawHLine_R(buf->light_plane, left, right, y, A_NORMAL);
			//GrayDrawSpan_LGRAY_R(left, right, addr);
			break;
		case COLOR_DARKGRAY:
			FastDrawHLine_R(buf->dark_plane, left, right, y, A_NORMAL);
			//GrayDrawSpan_DGRAY_R(left, right, addr);
			break;
		case COLOR_WHITE:
			//GrayDrawSpan_WHITE_R(left, right, addr);
			
			break;
	};
	#endif
	
	/*
	validate_segs();
	
	Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
	ClearGrayScreen2B(Vscreen0, Vscreen1);
	GrayDBufToggle();
	Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
	ClearGrayScreen2B(Vscreen0, Vscreen1);
	GrayDBufToggle();
	Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
	
	render_segs();
	GrayDBufToggle();
	*/
	
	
	
	//GrayDBufToggle();
	
	//ngetchx();
	//printf("wait\n");
	
	
	#if 0
	GrayDBufToggle();
	//GrayWaitNSwitches(20);
	
	volatile long i = 0;
	
	for(i = 0; i < 0xFFFF / 8; i++) ;
	
	GrayDBufToggle();
	Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
	#endif
	
	
}

inline void insert_seg(Seg *prev, Seg *seg) {
	Seg *save;

	save = prev->next;
	prev->next = seg;
	seg->next = save;
}

inline void remove_seg(Seg *prev) {
	prev->next = prev->next->next;
}

// Inserts a new segment, but tries to extend the previous segment, if possible
inline void extend_insert_seg(CBuffer* buf, Seg* prev, unsigned char left, unsigned char right) {
	if(right < left)
		return;
		
	char merge_left = prev->right + 1 >= left;
	char merge_right = prev->next && right == prev->next->left - 1;
	
	/*
	if(prev)
		merge_right = (right == prev->next->left - 1);
	else
		merge_right = (right == (*first)->left - 1);
	*/
	
	
	
	
	//c = 64 + (merge_left << 1) + merge_right;
	
	if(merge_left) {
		if(merge_right) {
			prev->right = prev->next->right;				
			remove_seg(prev);
		}
		else {
			// Just merging left
			prev->right = right;
		}
	}
	else if(merge_right) {
		// Just merging right
		prev->next->left = left;
	}
	else {
		Seg* s = alloc_seg(buf);
		s->left = left;
		s->right = right;
		insert_seg(prev, s);
	}
}

long seg_calls = 0;

inline void add_seg(CBuffer* buf, short y, short left_s, short right_s, short color) {
	draw_seg(buf, y, left_s, right_s, color);
	return;
	
	Seg* seg = buf->lines[y].start.next;		// Current segment we're looking at
	Seg* prev = &buf->lines[y].start;			// Segment before seg
	Seg* new_seg;				// The new segment we're going to add
	
	// Swap the endpoints if in the wrong order
#if 0
	if(right < left) {
		SWAP(left, right);
		SWAP(buf->pix_left_start, buf->pix_right_start);
		SWAP(buf->pix_left_dx, buf->pix_right_dx);
		
		//buf->pix_left_dx = -buf->pix_left_dx;
		//buf->pix_right_dx = -buf->pix_right_dx;
	}
#endif

	
	// Clip the endpoints
	if(left_s < 0)
		left_s = 0;
		
	if(right_s >= buf->width)
		right_s = buf->width - 1;
	
	if(left_s >= buf->width || right_s < 0)
		return;
	
	unsigned char left = left_s;
	unsigned char right = right_s;
	
	if(seg == NULL) {	//no segments in this scanline, so just add it
		new_seg = alloc_seg(buf);
		new_seg->left = left;
		new_seg->right = right;
		new_seg->next = NULL;
		prev->next = new_seg;
		draw_seg(buf, y, left, right, color);
		goto done;
	}
	
	//skips the segs we're not interested in
	while(seg->right < left - 1) {
		seg_calls++;
		prev = seg;
		seg = seg->next;
		
		if(seg == NULL) {		//no more segs left, so append it to the end
			extend_insert_seg(buf, prev, left, right);
			draw_seg(buf, y, left, right, color);
			goto done;
		}
	}

	
	//GrayDBufToggleSync();
	//printf("A");
	//GrayDBufToggleSync();
	
	if(left >= seg->left){
		if(right <= seg->right) {
			// The segment can be completely clipped away
			return;
		}
		else{
			// The segment begins in the middle of this segment, but continues past the end
			left = seg->right + 1;
			prev = seg;
			seg = seg->next;
		}
	}
	
	short new_left;
	short temp_right;
	
	while(left <= right) {
		seg_calls++;
		if(seg) {
			if(right <= seg->right - 1) {
				// Our end finishes before the end of the current seg
				if(right >= seg->left) {
					// Our end actually finishes inside the current segment
					right = seg->left - 1;
				}
				
				// Either way, we've reached the end of this segment
				new_left = right + 1;
				temp_right = right;
			}
			else {
				temp_right = seg->left - 1;
				new_left = seg->right + 1;
			}
		}
		else {
			new_left = right + 1;
			temp_right = right;
		}
		
		
		extend_insert_seg(buf, prev, left, temp_right);
		draw_seg(buf, y, left, temp_right, color);
		
		left = new_left;
		prev = seg;
		seg = seg->next;
	}
done:
	buf->lines[y].full = buf->lines[y].start.left == 0 && buf->lines[y].start.right == buf->width - 1;
	//(buf->lines[y].start && buf->lines[y].start->left == 0 && buf->lines[y].start->right == buf->width - 1);
	
	buf->lines_left -= buf->lines[y].full;
}
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
#if 0
		
		|| right <= seg->right - 1) {
			if(seg && right >= seg->left) {
				right = seg->left;
		}
		else if(
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
	}
		if(prev && left - 1 == prev->right){
			new_seg = prev;
			//left = prev->left;
		}
		else{
			new_seg = alloc_seg();
			new_seg->left = left;
			//new_seg->left = left;
			//new_seg->right = right;
		
			//insert_seg(&head_ptr[y],prev,new_seg);
		}
			
		if(seg == NULL || right < seg->right - 1) {
			// Nothing left to clip against, so just add it
			if(seg) {
				if(right >= seg->left) {
					right = seg->left;
					
					new_seg->left = seg->left;
					Seg* next = seg->next;
					remove_seg(&head_ptr[y], prev, new_seg);
					new_seg->next = next;
				}
			}
			else {
				new_seg->right = right;
				insert_seg(&head_ptr[y], prev, new_seg);
			}					
				
			draw_seg(y, left, right, color);
			return;
		}
		else {
			new_seg->right = seg->left - 1;
		}
		
		
			
		
		
		

















		if(seg == NULL||right < seg->right-1){
			draw_seg(y,left,right,color);
			new_seg->right = right;
			return;
		}
		else{
			draw_seg(y,left,seg->left-1,color);
			new_seg->right = seg->right;
			
			if(prev != NULL)
				remove_seg(prev);
			//else {
			//	head_ptr[y] = new_seg;
			//}
			
			left = seg->right+1;
			prev = seg;
			seg = seg->next;
		}
	}
	//GrayDBufToggleSync();
	//printf("B\n");
	//GrayDBufToggleSync();
}

#endif