// ==================================================================
// Filename:	Nimage.h
//
// Date:        01/11/95
// Author:      Neil Sumpter
//              (adpated from D.M.Sergeant)
// Description:
//	General image class, includes data storage, and 
//	display of image. Also uses stuff for live capture
//
// $Id: image.h,v 1.2 2000-12-01 22:08:18 vaughan Exp $
// RTV
// ==================================================================

#ifndef _NIMAGE_CLASS_
#define _NIMAGE_CLASS_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


unsigned int RGB( int r, int g, int b );

/*const int red     = 0x00FF0000;
const int green   = 0x0000FF00;
const int blue    = 0x005050FF;
const int magenta = 0x00FF00FF;
const int cyan    = 0x0000FFFF;
const int white   = 0x00FFFFFF;
const int black   = 0x00000000;
const int yellow  = 0x00FFFF00;
*/

typedef struct Point
{
  int x, y;
};

typedef struct Rect
{
  int toplx, toply, toprx, topry, botlx, botly, botrx, botry;
};

class Nimage
{
public:
        int	width;
	int	height;
	
	unsigned char*   data;
	char*	win_title;
	
	Nimage(unsigned char* array, int w,int h);
	Nimage(int w,int h);
	Nimage(Nimage*);
	Nimage(char* fname);

	inline	int	get_width(void)		{return width;}
	inline	int	get_height(void)	{return height;}
	inline	int	get_size(void)		{return width*height;}

	inline unsigned char get_pixel(int x, int y)
	  { 
	    if (x<0 || x>=width || y<0 || y>=height) return 0;
	    return (char)data[x+(y*width)]; 
	  }
	
	inline void set_pixel(int x, int y, unsigned char c)
	  {
	    if (x<0 || x>=width || y<0 || y>=height) return;
	    data[(x + y*width)] = c;
	  }
	
	inline	unsigned char *get_data(void)	{return data;}

	void	copy_from(Nimage* img);

	int N_draw_big;
	void	draw_big(void);
	void	draw_small(void);
	void    bgfill( int x, int y, unsigned char col );
	void	draw_box(int,int,int,int,unsigned char);
	void	draw_circle(int x,int y,int r,unsigned char c);
	void	draw_line(int x1,int y1,int x2,int y2,unsigned char c);

	// new 1 Dec 2000 - RTV
	void	draw_rect( const Rect t, unsigned char c);

	//void	draw_line_detect(int x1,int y1,int x2,int y2,int c);	
	unsigned char line_detect(int x1,int y1,int x2,int y2,
				  unsigned char c );
	unsigned char rect_detect( const Rect& r, 
				   unsigned char c );
	void	clear(unsigned char i);
	void	save_image_as_ppm(char*,char*);
	void	save_raw(char* fname);
	void	load_raw(char* fname);
	void	load_pnm(char* fname);
	//char*   color_name( int col );

	~Nimage(void);
};



#endif
