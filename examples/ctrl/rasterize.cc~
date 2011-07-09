/**
 * File: rasterize.cc
 * Description: example of how to use the Stg::Model::Rasterize() method
 * Date: 12 June 2009
 * Author: Richard Vaughan
*/

// uncomment the line 'ctrl "rasterize"' in <stage source>/worlds/simple.world to try this example

#include "stage.hh"
using namespace Stg;


// Stage calls this when the model starts up
extern "C" int Init( Model* mod )
{  
  const unsigned int dw = 40, dh = 20;
  
  uint8_t* data = new uint8_t[dw*dh*2];
  memset( data, 0, sizeof(uint8_t) * dw * dh );
  
  printf( "\n[Rasterize] Raster of model \"%s\":\n", 
	  mod->Token() );
  
  Size modsz( mod->GetGeom().size );

  mod->Rasterize( data, dw, dh, 
		  modsz.x / (float)dw, 
		  modsz.y / (float)dh );

  for( unsigned int y=0; y<dh; y++ )
    {
      printf( "[Rasterize] " );
      for( unsigned int x=0; x<dw; x++ )
	putchar( data[x + ((dh-y-1)*dw)] ? 'O' : '.' );
      putchar( '\n' );
    }  
  delete data;
  puts( "[Rasterize] Done" );

  return 0; //ok
}



