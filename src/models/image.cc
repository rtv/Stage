/*************************************************************************
 * image.cc - bitmap image class Nimage with processing functions
 *            originally by Neil Sumpter and others at U.Leeds, UK.
 * RTV
 * $Id: image.cc,v 1.1.4.1.2.1 2004-02-17 17:19:49 gerkey Exp $
 ************************************************************************/

#include <math.h>
#include <assert.h>
#include <ctype.h>

#include <iostream>
#include <fstream>

using namespace std;

#include "stage_types.hh"
#include "image.hh"

//#if INCLUDE_ZLIB
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif


//#define DEBUG

typedef short           short_triple[3];

unsigned int RGB( int r, int g, int b )
{
  unsigned int res;
  //cout << "RGB!" << endl;
  res = (((r << 8) | g) << 8) | b;
  return res;
}

unsigned char aget_pixel(unsigned char* data, int width, int height, int x, int y)
{ 
  if (x<0 || x>=width || y<0 || y>=height) return 0;
  return (unsigned char)data[x+(y*width)]; 
}

void aset_pixel(unsigned char* data, int width, int height, int x, int y, unsigned char c)
{
  if (x<0 || x>=width || y<0 || y>=height) return;
  data[(x + y*width)] = c;
}

// Default constructor
Nimage::Nimage()
{
    width = 0;
    height = 0;
    data = NULL;
}


// constuct by copying an existing image
Nimage::Nimage(Nimage* img)
{
#ifdef DEBUG
  cout << "Image: " << width << 'x' << height << flush;
#endif

  width = img->get_width();
  height = img->get_height();
  data = new unsigned char[width*height];
  
  this->copy_from(img);
 
#ifdef DEBUG
  cout << '@' << data << endl;
#endif
}

// construct from width / height 
Nimage::Nimage(int w,int h)
{
#ifdef DEBUG
  cout << "Image: " << width << 'x' << height << flush;
#endif
  
  width = w; height = h;
  data 	= new unsigned char[width*height];

#ifdef DEBUG
  cout << "unsigned char: " << sizeof( unsigned char ) << endl;
  cout << '@' << data << endl;
#endif
  //cout << "constructed NImage" << endl;
}

// construct from width / height with existing data
Nimage::Nimage(unsigned char* array, int w,int h)
{
#ifdef DEBUG
  cout << "Image: " << width << 'x' << height << flush;
#endif

  width 	= w;
  height 	= h;
  data 		= array;

#ifdef DEBUG
  cout << '@' << data << endl;
#endif
}

//destruct
Nimage::~Nimage(void) 	
{
	if (data) delete [] data;
	data = NULL;
}

void Nimage::copy_from(Nimage* img)
{
  if ((width != img->width)||(height !=img->height))
    {
      fprintf(stderr,"Nimage:: mismatch in size (copy_from)\n");
      exit(0);
    }
  
  memcpy(data,img->data,width*height);
}



void Nimage::bgfill( int x, int y, unsigned char col )
{
  // simple recursive flood fill
  if( x < 0 || x >= width ) return;
  if( y < 0 || y >= height ) return;

  set_pixel( x,y,col );
 
  if( get_pixel( x,y-1 ) != col ) bgfill( x, y-1, col );
  if( get_pixel( x,y+1 ) != col ) bgfill( x, y+1, col );
  if( get_pixel( x+1,y ) != col ) bgfill( x+1, y, col );
  if( get_pixel( x-1,y ) != col ) bgfill( x-1, y, col );
}

 
void Nimage::draw_box(int x,int y,int w,int h,unsigned char col)
{
	int i;

	for (i=0;i<w;i++)
	{
		data[x+i+y*width]=col;
		data[x+i+(y+h)*width]=col;
	}

	for (i=0;i<h;i++)
	{
		data[x+(y+i)*width]=col;
		data[x+w+(y+i)*width]=col;
	}
}

void Nimage::save_raw(char* fname)
{
  // this will not write files readable by load_raw!
  FILE *stream = fopen( fname, "wb");
  fwrite( this->get_data(), 1, width*height, stream );
  fclose(stream);
}

void Nimage::load_raw(char* fname)
{
  int n, m;
  char p;

  FILE *stream = fopen( fname, "r");

  for( n=0; n<height; n++ )
    for( m=0; m<width; m++ )
      {
	fread( &p, 1, 1, stream );
	set_pixel( m,n, p );
      }

  fclose(stream);
}


// Load a pnm image.
bool Nimage::load_pnm(const char* fname)
{
  char magicNumber[10];
  //char comment[512];
  int whiteNum;

  ifstream source( fname );

  if( !source )
  {
    PRINT_ERR1("unable to open image file [%s]", fname);
    return false;
  }

  source >> magicNumber;

  if( strcmp(magicNumber, "P5" ) != 0 )
  {
    PRINT_ERR("image file is of incorrect type:"
              " should be pnm, binary, monochrome (magic number P5)");
    return false; 
  }
  
  // ignore the end of this line
  source.ignore( 1024, '\n' );
  
  // ignore comment lines
  while( source.peek() == '#' )
    source.ignore( 1024, '\n' );  

  // get the width, height and white value  
  source >> width >> height >> whiteNum;

  // skip to the end of the line again
  source.ignore( 1024, '\n' );

  unsigned int numPixels = width * height;

  // make space for the pixels
  if (data) delete[] data;
  data = new unsigned char[ numPixels ];
  
  source.read( (char*)data, numPixels );

  // check that we read the right amount of data
  assert( (unsigned int)(source.gcount()) == numPixels );

  source.close();

  return true;
}


// Load a gzipped pnm file
bool Nimage::load_pnm_gz(const char* fname)
{
//#ifndef INCLUDE_ZLIB
#ifndef HAVE_LIBZ
  printf("opening %s\n", fname);
  printf("gzipped files not supported\n");
  return false;
#else
    
  char line[1024];
  char magicNumber[10];
  //  char comment[256];
  int whiteNum; 

#ifdef DEBUG
  printf("opening %s\n", fname);
#endif

  gzFile file = gzopen(fname, "r");
  if (file == NULL)
  {
    PRINT_ERR1("unable to open image file [%s]", fname);
    return false;
  }

  // Extremely crude and ugly file parsing! Fix sometime.  Andrew.
  // yeah - and it had a yukky bug. fixed that but it's still a crude
  // parser - still the format definition for PNMs is strict, so we're
  // almost certainly OK. - rtv

  // (some weeks later) ok, so some programs don't insert a comment line into
  // pnm files, so I'm detecting comment lines now.
  // should change this to use libpnm in the near future - rtv

  // Read and check the magic number
  //
  gzgets(file, magicNumber, sizeof(magicNumber));
  if (strcmp(magicNumber, "P5\n") != 0)
  {
    PRINT_ERR("image file is of incorrect type:"
              " should be pnm, binary, monochrome (magic number P5)");
    return false;
  }
  
  //gzgets(file, comment, sizeof(comment));
  
  do{
    gzgets(file, line, sizeof(line)); 
  } while(!isdigit(line[0]));
      
  sscanf(line, "%d %d", &width, &height );

  gzgets(file, line, sizeof(line));
  sscanf(line, "%d", &whiteNum);

  if (data)
    delete[] data;
  data = new unsigned char[ width * height ];
  
  // zero the image
  memset( data, 0, width * height * sizeof( data[0] ) );

  for(int n=0; n<height; n++ )
  {
    for(int m=0; m<width; m++ )
    {
      unsigned char a = gzgetc(file);
      if (a > 0)
	set_pixel( m,n, 1 );
    }
  }

  gzclose(file);

  return true;
#endif
}


// Load from an xfig file
// Note that we need both the pixels-per-meter and the figure scale
// to create an image of the correct size and scale.
bool Nimage::load_fig(const char* filename, double ppm, double scale)
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("unable to open image file\n");
    return false;
  }

  char line[1024];
  int format;

  if (!fgets(line, sizeof(line), file))
  {
    printf("unexpected end-of-file\n");
    fclose(file);
    return false;
  }

  // Read the file format
  if (strncmp(line, "#FIG 3.2", 8) == 0)
    format = 32;
  else
  {
    printf("unrecognized file format\n");
    fclose(file);
    return false;
  }

  // Read the header and discard most of it
  // The <units> variable is used to handle imperial vs metric units.
  // When using imperial units, xfig will use 1200 pixels/inch.
  // When using metric units, xfig will use 450 pixels/cm.
  // This latter figure is completely undocumented; its just a magic
  // number I derived empirically.  ahoward
  double units = 1200 / 0.0254;
  for (int i = 0; i < 8;)
  {
    if (fgets(line, sizeof(line), file) == NULL)
      break;
    if (line[0] == '#')
      continue;
    if (i == 2)
    {
      if (strncmp(line, "Metric", 6) == 0)
        units = 450 / 0.01;
    }
    i++;
  }

  // Create the figure
  // We assume the whole page should be included,
  // and that we are using US letter in landscape mode.
  this->width = (int) (ppm * 11.5 * 0.0254 * scale);
  this->height = (int) (ppm * 8.5 * 0.0254 * scale);
  if (this->data)
      delete[] this->data;
  this->data = new unsigned char[ this->width * this->height ];

  // Read the objects
  while (true)
  {
    if (fgets(line, sizeof(line), file) == NULL)
      break;
    if (line[0] == '#')
      continue;

    int code = atoi(line);
    if (code == 2)
        load_fig_polyline(file, line, sizeof(line), ppm / units * scale);
    else
        printf("warning : unsupported object type in figure\n");
  }
  
  fclose(file);
  return true;
}


// Read in a polyline figure
bool Nimage::load_fig_polyline(FILE *file, char *line, int size, double scale)
{
  // Read object header and discard most of it  
  strtok(line, " \t");
  for (int i = 0; i < 14; i++)
    strtok(NULL, " \t");
  int npoints = atoi(strtok(NULL, " \t"));

  for (int i = 0; i < npoints;)
  {
    if (fgets(line, size, file) == NULL)
      break;
    if (line[0] == '#')
      continue;

    char *t1, *t2;
    int ax=0, ay=0, bx=0, by=0;
    
    t1 = strtok(line, " \t\n");
    while (i < npoints)
    {
      t2 = strtok(NULL, " \t\n");      
      bx = (int) (atoi(t1) * scale);
      by = (int) (atoi(t2) * scale);
      i++;
      if (i >= 2)
          draw_line(ax, ay, bx, by, 1);
      ax = bx;
      ay = by;
      t1 = strtok(NULL, " \t\n");
      if (!t1)
          break;
    }
  }

  return true;
}


void Nimage::draw_big(void)
{
  N_draw_big = true;
}

void Nimage::draw_small(void)
{
  N_draw_big = false;
}

void Nimage::draw_circle(int x,int y,int r,unsigned char c)
{
  double i,cx,cy;
  int x1,y1,x2,y2;
  
  x1=x;y1=y+r;	
  
  for (i=0;i<2.0*M_PI;i+=0.1)
    {
      cx = (double) x + (double (r) * sin(i));
      cy = (double) y + (double (r) * cos(i));
      x2=(int)cx;y2=(int)cy;
      draw_line(x1,y1,x2,y2,c);
      x1=x2;y1=y2;
    }	
  
  draw_line(x1,y1,x,y+r,c);
}

void Nimage::draw_rect( const Rect t, unsigned char c)
{
  draw_line( t.toplx, t.toply, t.toprx, t.topry, c );
  draw_line( t.toprx, t.topry, t.botlx, t.botly, c );
  draw_line( t.botlx, t.botly, t.botrx, t.botry, c );
  draw_line( t.botrx, t.botry, t.toplx, t.toply, c );
}


void Nimage::draw_line(int x1,int y1,int x2,int y2,unsigned char col)
{
  int delta_x, delta_y;
  int delta, incE, incNE;
  int x, y;
  int neg_slope = 0;
  
  if (x1 > x2)
    {
      delta_x = x1 - x2;
      if (y1 > y2)	delta_y = y1 - y2;
      else		delta_y = y2 - y1;
      
      if (delta_y <= delta_x)	draw_line(x2, y2, x1, y1, col);
    }
  if (y1 > y2)
    {
      delta_y = y1 - y2;
      if (x1 > x2)	delta_x = x1 - x2;
      else		delta_x = x2 - x1;
      
      if (delta_y > delta_x)	draw_line(x2, y2, x1, y1, col);
    }
  
  if (x1 > x2)
    {
      neg_slope = 1;
      delta_x = x1 - x2;
    }
  else
    delta_x = x2 - x1;
  
  if (y1 > y2)
    {
      neg_slope = 1;
      delta_y = y1 - y2;
    }
  else
    delta_y = y2 - y1;
  
  x = x1;
  y = y1;
  
  set_pixel(x,y,col);
  
  if (delta_y <= delta_x)
    {
      delta = 2 * delta_y - delta_x;
      incE = 2 * delta_y;
      incNE = 2 * (delta_y - delta_x);
      
      while (x < x2)
	{
	  if (delta <= 0)
	    {
	      delta = delta + incE;
	      x++;
	    }
	  else
	    {
	      delta = delta + incNE;
	      x++;
	      if (neg_slope)	y--;
	      else		y++;
	    }
	  set_pixel(x,y,col);
	}
    }
  else
    {
      delta = 2 * delta_x - delta_y;
      incE = 2 * delta_x;
      incNE = 2 * (delta_x - delta_y);
      
      while (y < y2) 
	{
	  if (delta <= 0)
	    {
	      delta = delta + incE;
	      y++;
	    }
	  else
	    {
	      delta = delta + incNE;
	      y++;
	      if (neg_slope)	x--;
	      else		x++;
	    }
	  set_pixel(x,y,col);
	}
    }
}

unsigned char Nimage::rect_detect( const Rect& r)
{
  unsigned char hit;
  
  if( (hit = line_detect( r.toplx, r.toply, r.toprx, r.topry)) > 0 ) 
    return hit;
  
  if( (hit = line_detect( r.toprx, r.topry, r.botlx, r.botly)) > 0 )
    return hit;
  
  if( (hit = line_detect( r.botlx, r.botly, r.botrx, r.botry)) > 0 )
    return hit;
  
  if( (hit = line_detect( r.botrx, r.botry, r.toplx, r.toply)) > 0 )
    return hit;

  return 0;
}

unsigned char Nimage::line_detect(int x1,int y1,int x2,int y2)
{
  int delta_x, delta_y;
  int delta, incE, incNE;
  int x, y;
  int neg_slope = 0;
  unsigned char pixel;
  

  if (x1 > x2)
    {
      delta_x = x1 - x2;
      if (y1 > y2)	delta_y = y1 - y2;
      else		delta_y = y2 - y1;
      
      if (delta_y <= delta_x) return( line_detect(x2, y2, x1, y1));
    }
  if (y1 > y2)
    {
      delta_y = y1 - y2;
      if (x1 > x2)	delta_x = x1 - x2;
      else		delta_x = x2 - x1;
      
      if (delta_y > delta_x) return( line_detect(x2, y2, x1, y1));
    }
  
  if (x1 > x2)
    {
      neg_slope = 1;
      delta_x = x1 - x2;
    }
  else
    delta_x = x2 - x1;
  
  if (y1 > y2)
    {
      neg_slope = 1;
      delta_y = y1 - y2;
    }
  else
    delta_y = y2 - y1;
  
  x = x1;
  y = y1;
  
  //check to see if this pixel is an obstacle
  pixel = get_pixel( x,y );
  if( pixel != 0)
   { 
     return pixel;
   }

  //set_pixel(x,y,col);
  
  if (delta_y <= delta_x)
    {
      delta = 2 * delta_y - delta_x;
      incE = 2 * delta_y;
      incNE = 2 * (delta_y - delta_x);
      
      while (x < x2)
	{
	  if (delta <= 0)
	    {
	      delta = delta + incE;
	      x++;
	    }
	  else
	    {
	      delta = delta + incNE;
	      x++;
	      if (neg_slope)	y--;
	      else		y++;
	    }

	  //check to see if this pixel is an obstacle
	  pixel = get_pixel( x,y );
	  if( pixel != 0)
	    { 
	      return pixel;
	    }
	  
	  //set_pixel(x,y,col);
	}
    }
  else
    {
      delta = 2 * delta_x - delta_y;
      incE = 2 * delta_x;
      incNE = 2 * (delta_x - delta_y);
      
      while (y < y2) 
	{
	  if (delta <= 0)
	    {
	      delta = delta + incE;
	      y++;
	    }
	  else
	    {
	      delta = delta + incNE;
	      y++;
	      if (neg_slope)	x--;
	      else		x++;
	    }
	  //check to see if this pixel is an obstacle
	  pixel = get_pixel( x,y );
	  if( pixel != 0)
	    { 
	      return pixel;
	    }
	  //set_pixel(x,y,col);
	}
    }
  return 0;
}


void Nimage::clear(unsigned char col)
{
  //cout << "Clear: " << data << ',' << col << ',' << width*height << endl;
  memset(data,col,width*height*sizeof(unsigned char));
}


/* save out image data to a ppm (colour pbm) image file */
/* Parameters: IN fname, the name of the file to save the image data to */
/*                cmap, file containing the mapping of grey scale to colour. */
// default value cmap = "/home/dream/derek/bin/high_col";

void Nimage::save_image_as_ppm(char *fname, char *cmap)
{
    FILE *f_out_ptr;
	int bytes_written;
	unsigned char *im_ptr;
	short_triple *the_cmap;

	f_out_ptr = fopen(fname, "wb");

	if (!f_out_ptr)
	{
	fprintf(stderr,"::save_ppm_data, cannot write to file %s\n", fname);
	fflush(stderr);
	exit(0);
	}

	bytes_written = 256;
	the_cmap = new short_triple [bytes_written];

	for (int i = 0; i < bytes_written; i++)
	{
			the_cmap[i][0]=i;
			the_cmap[i][1]=i;
			the_cmap[i][2]=i;
	}

	the_cmap[1][0]=255;the_cmap[1][1]=0;the_cmap[1][2]=0;
	the_cmap[2][0]=0;the_cmap[2][1]=255;the_cmap[2][2]=0;
	the_cmap[3][0]=255;the_cmap[3][1]=255;the_cmap[3][2]=0;
	the_cmap[4][0]=0;the_cmap[4][1]=0;the_cmap[4][2]=255;
	the_cmap[5][0]=255;the_cmap[5][1]=0;the_cmap[5][2]=255;
	the_cmap[6][0]=0;the_cmap[6][1]=255;the_cmap[6][2]=255;
	the_cmap[7][0]=255;the_cmap[7][1]=255;the_cmap[7][2]=255;

  /***** First write the PPM header data *******/
  /* need to save these variables */
  fprintf(f_out_ptr, "P6\n");
  fprintf(f_out_ptr, "# PPM raw bitmap image file format\n");
  fprintf(f_out_ptr, "#%s\n", cmap);    // To indicate where to find colour set

  fprintf(f_out_ptr, "# Width x Height\n");
  fprintf(f_out_ptr, "%d %d\n", width, height);

  fprintf(f_out_ptr, "# Max Grey number\n");
  fprintf(f_out_ptr, "255\n");

/*** u_char is defnd in the read_pgm_data fn (makes fwrite line shorter). ***/

  /***** Then write out the raw image data *****/

  bytes_written = 0;

  for (im_ptr = data; im_ptr < data + (width*height); im_ptr++)
  {
    fputc(char(the_cmap[*im_ptr][0]), f_out_ptr);
    fputc(char(the_cmap[*im_ptr][1]), f_out_ptr);
    fputc(char(the_cmap[*im_ptr][2]), f_out_ptr);
  }

  fflush(f_out_ptr); fclose(f_out_ptr);
}





