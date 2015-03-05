// C Source File
// Created 2/17/2015; 12:05:12 AM

#include <tigcclib.h>

#include "3D.h"

extern short *sin_tab;

void mul_mat3x3(Mat3x3* a, Mat3x3* b, Mat3x3* res)
{
    int i, j, k;
    
    for(i = 0; i < 3; i++)
    	for(j = 0; j < 3; j++)
    		(*res)[i][j] = 0;
    
    for(i = 0; i < 3; i++)
    {
            for(j = 0; j < 3; j++)
            {
                    for(k = 0; k < 3; k++)
                    {
                            (*res)[i][j] += mul_fps_frac((*a)[i][k], (*b)[k][j]);
                    }
            }
    }
}

void x3d_construct_mat3(Vex3D *angle, Mat3x3 *dest) {
	short cos_angle_y = 90 - angle->y;
	
	if(cos_angle_y < 0)
		cos_angle_y += 360;
		
	short sin_y = sin_tab[angle->y];
	short cos_y = sin_tab[cos_angle_y];
	
	
	short cos_angle_x = 90 - angle->x;
	
	if(cos_angle_x < 0)
		cos_angle_x += 360;
		
	short sin_x = sin_tab[angle->x];
	short cos_x = sin_tab[cos_angle_x];
	
	
#if 0
	Mat3x3 mat = {
		{ cos_y, 0, -sin_y},
		{ 0, 32767, 0 }, 
		{ sin_y, 0, cos_y}
	};
#else
	Mat3x3 mat_y = {
		{ cos_y, 0, sin_y}, 
		{ 0, 32767, 0}, 
		{ -sin_y, 0, cos_y}
	};
#endif
	
	Mat3x3 mat_x = {
		{32767, 0, 0},
		{0, cos_x, -sin_x},
		{0, sin_x, cos_x},
	};
	
	Mat3x3 mul_res;
	
	mul_mat3x3(&mat_x, &mat_y, &mul_res);
	
	memcpy(dest, &mul_res, sizeof(Mat3x3));
}


// Ewwwww gross!!! Circa 2012 code!
void x3d_construct_mat3_old(Vex3D *angle, Mat3x3 *dest) {
	//we use the product to sum identities (where efficient) instead of multiplication
	//variables are in this format:
	//[(s)ine or (c)osine]_[angle][(p)lus or (m)inus][angle]

	//short c_ymz,c_ypz;
	//short s_ymz,s_ypz;
	
//	short c_xmz,c_xpz;

	//cos(z)*cos(y)+sin(z)*sin(x)*sin(y)
	short c_ymz = 90-(angle->y-angle->z);
	short c_ypz = 90-(angle->y+angle->z);
	
	while(c_ypz>=360){
		c_ypz-=360;
	}
	while(c_ymz>=360){
		c_ymz-=360;
	}
	
	while(c_ypz<0){
		c_ypz+=360;
	}
	while(c_ymz<0){
		c_ymz+=360;
	}
	
	//printf("-[%d] +[%d]\n",c_ymz,c_ypz);
	
	short xxx = ((long)sin_tab[c_ymz]-sin_tab[c_ypz])>>1;
	
	(*dest)[0][0] = (((long)sin_tab[c_ymz]+sin_tab[c_ypz])>>1)+(mul_fps_frac(xxx,sin_tab[angle->x]));
	
	//-sin(z)*cos(x)
	short s_xmz = angle->x-angle->z;
	short s_xpz = angle->x+angle->z;
	
	if(s_xmz<0){
		s_xmz+=360;
	}
	if(s_xpz>=360){
		s_xpz-=360;
	}
	(*dest)[0][1] = ((long)sin_tab[s_xmz]-sin_tab[s_xpz])>>1;
	
	//-cos(z)*sin(y)+sin(z)*sin(x)*cos(y)
	short s_ymz = angle->y-angle->z;
	short s_ypz = angle->y+angle->z;
	
	if(s_ymz<0){
		s_ymz+=360;
	}
	if(s_ypz>=360){
		s_ypz-=360;
	}
	
	(*dest)[0][2] = ((-(long)sin_tab[s_ypz]-sin_tab[s_ymz])>>1)+mul_fps_frac(((long)sin_tab[s_ypz]-sin_tab[s_ymz])>>1,sin_tab[angle->x]);
	
	
	
	
	
	
	
	
	//sin(z)*cos(y)-cos(z)*sin(x)*sin(y) 
	
	short czsxsy = mul_fps_frac(sin_tab[angle->x],((long)sin_tab[s_ypz]+sin_tab[s_ymz])>>1);
	
	short szcy = ((long)sin_tab[s_ypz]-sin_tab[s_ymz])>>1;
	
	(*dest)[1][0] = szcy-czsxsy;
	
	
	
	
	
	
	
	
	
	
	
	//cos(z)*cos(x)
	short c_xmz = 90-s_xmz;
	short c_xpz = 90-s_xpz;
	
	if(c_xpz>=360){
		c_xpz-=360;
	}
	if(c_xmz>=360){
		c_xmz-=360;
	}
	if(c_xpz<0){
		c_xpz+=360;
	}
	if(c_xmz<0){
		c_xmz+=360;
	}
	
	(*dest)[1][1] = ((long)sin_tab[c_xmz]+sin_tab[c_xpz])>>1;
	
	//-sin(z)*sin(y)-cos(z)*sin(x)*cos(y)
	(*dest)[1][2] = (long)(((long)sin_tab[c_ypz]-(long)sin_tab[c_ymz])>>1)-(long)mul_fps_frac(((long)sin_tab[c_ypz]+sin_tab[c_ymz])>>1,sin_tab[angle->x]);
	
	//cos(x)*sin(y)
	short s_xmy = angle->x-angle->y;
	short s_xpy = angle->x+angle->y;
	
	if(s_xmy<0){
		s_xmy+=360;
	}
	if(s_xpy>=360){
		s_xpy-=360;
	}
	
	(*dest)[2][0] = ((long)sin_tab[s_xpy]-sin_tab[s_xmy])>>1;
	
	//sin(x)
	(*dest)[2][1] = sin_tab[angle->x];
	
	//cos(x)*cos(y)
	short c_xmy = 90-s_xmy;
	short c_xpy = 90-s_xpy;
	
	if(c_xmy>=360){
		c_xmy-=360;
	}
	if(c_xpy>=360){
		c_xpy-=360;
	}
	
	if(c_xpy<0){
		c_xpy+=360;
	}
	if(c_xmy<0){
		c_xmy+=360;
	}
	
//	printf("(%d,%d)\n",c_xmy,c_xpy);
//	ngetchx();

	(*dest)[2][2] = ((long)sin_tab[c_xpy]+sin_tab[c_xmy])>>1;
	
}










































