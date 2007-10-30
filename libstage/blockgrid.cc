#include "stage.hh"

StgBlockGrid::StgBlockGrid( uint32_t width, uint32_t height )
{
  printf( "Creating StgBlockGrid of [%d,%d] elements\n", 
	  width, height );
  
  this->width = width;
  this->height = height;
  
  this->lists = new GSList*[width * height];
  bzero( lists, width*height * sizeof(GSList*));
}    

StgBlockGrid::~StgBlockGrid()
{
  delete[] lists;
}

void StgBlockGrid::AddBlock( uint32_t x, uint32_t y, StgBlock* block )
{
  if( x < width && y < height )
    {  
      unsigned int index = x + width*y;
      lists[index] = g_slist_prepend( lists[index], block );
      block->RecordRenderPoint( x, y );
    }
}


GSList* StgBlockGrid::GetList( uint32_t x, uint32_t y )
{
  if( x < width && y < height )
    {  
      unsigned int index = x + width*y;
      return lists[index];
    }
  return NULL;
}

void StgBlockGrid::RemoveBlock( uint32_t x, uint32_t y, StgBlock* block )
{
  //printf( "remove block %u %u\n", x, y );

  if( x < width && y < height )
    {
      unsigned int index = x + width*y;
      lists[index] = g_slist_remove( lists[index], block );
    }
}

void StgBlockGrid::RemoveBlock( StgBlock* block )
{
  // todo - speed this up a great deal
  for( uint32_t x=0; x<width; x++ )
    for( uint32_t y=0; y<height; y++ )
      RemoveBlock(x,y,block );    
}

void StgBlockGrid::Draw()
{
  for( uint32_t x=0; x<width; x++ )
    for( uint32_t y=0; y<height; y++ )
      if( lists[ x + width*y] )
	glRecti( x,y,x+1,y+1 );
  
  //printf( "printing %d %d %d \n", x, y, index );
  
  //glTranslatef( x,y,0 );
    
  // 	    glBegin(GL_QUADS);		// Draw The Cube Using quads
  // 	    glColor3f(0.0f,1.0f,0.0f);	// Color Blue
  // 	    glVertex3f( 1.0f, 1.0f,-1.0f);	// Top Right Of The Quad (Top)
  // 	    glVertex3f(-1.0f, 1.0f,-1.0f);	// Top Left Of The Quad (Top)
  // 	    glVertex3f(-1.0f, 1.0f, 1.0f);	// Bottom Left Of The Quad (Top)
  // 	    glVertex3f( 1.0f, 1.0f, 1.0f);	// Bottom Right Of The Quad (Top)
  // 	    glColor3f(1.0f,0.5f,0.0f);	// Color Orange
  // 	    glVertex3f( 1.0f,-1.0f, 1.0f);	// Top Right Of The Quad (Bottom)
  // 	    glVertex3f(-1.0f,-1.0f, 1.0f);	// Top Left Of The Quad (Bottom)
  // 	    glVertex3f(-1.0f,-1.0f,-1.0f);	// Bottom Left Of The Quad (Bottom)
  // 	    glVertex3f( 1.0f,-1.0f,-1.0f);	// Bottom Right Of The Quad (Bottom)
  // 	    glColor3f(1.0f,0.0f,0.0f);	// Color Red	
  // 	    glVertex3f( 1.0f, 1.0f, 1.0f);	// Top Right Of The Quad (Front)
  // 	    glVertex3f(-1.0f, 1.0f, 1.0f);	// Top Left Of The Quad (Front)
  // 	    glVertex3f(-1.0f,-1.0f, 1.0f);	// Bottom Left Of The Quad (Front)
  // 	    glVertex3f( 1.0f,-1.0f, 1.0f);	// Bottom Right Of The Quad (Front)
  // 	    glColor3f(1.0f,1.0f,0.0f);	// Color Yellow
  // 	    glVertex3f( 1.0f,-1.0f,-1.0f);	// Top Right Of The Quad (Back)
  // 	    glVertex3f(-1.0f,-1.0f,-1.0f);	// Top Left Of The Quad (Back)
  // 	    glVertex3f(-1.0f, 1.0f,-1.0f);	// Bottom Left Of The Quad (Back)
  // 	    glVertex3f( 1.0f, 1.0f,-1.0f);	// Bottom Right Of The Quad (Back)
  // 	    glColor3f(0.0f,0.0f,1.0f);	// Color Blue
  // 	    glVertex3f(-1.0f, 1.0f, 1.0f);	// Top Right Of The Quad (Left)
  // 	    glVertex3f(-1.0f, 1.0f,-1.0f);	// Top Left Of The Quad (Left)
  // 	    glVertex3f(-1.0f,-1.0f,-1.0f);	// Bottom Left Of The Quad (Left)
  // 	    glVertex3f(-1.0f,-1.0f, 1.0f);	// Bottom Right Of The Quad (Left)
  // 	    glColor3f(1.0f,0.0f,1.0f);	// Color Violet
  // 	    glVertex3f( 1.0f, 1.0f,-1.0f);	// Top Right Of The Quad (Right)
  // 	    glVertex3f( 1.0f, 1.0f, 1.0f);	// Top Left Of The Quad (Right)
  // 	    glVertex3f( 1.0f,-1.0f, 1.0f);	// Bottom Left Of The Quad (Right)
  // 	    glVertex3f( 1.0f,-1.0f,-1.0f);	// Bottom Right Of The Quad (Right)
  // 	    glEnd();			// End Drawing The Cube
  
	    //glTranslatef( -x, -y, 0 );
} 
