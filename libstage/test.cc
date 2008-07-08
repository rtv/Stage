#include "stage.hh"

/* libstage test program */

using namespace Stg;

const double epsilon = 0.000001;

void interact( StgWorldGui* wg )
{
  //for( int i=0; i<100; i++ )
  // wg->Cycle();

  wg->Update();
}

void test( const char* str, double a, double b )
{
  if( fabs(a-b) > epsilon )
	 printf( "FAIL %s expected %.3f saw %.3f\n", str, a, b );  
}

void test( const char* str, stg_pose_t a, stg_pose_t b )
{
  if( fabs(a.x-b.x) > epsilon )
	 printf( "POSE FAIL %s expected pose.x %.3f saw pose.x %.3f\n", str, a.x, b.x );  
  if( fabs(a.y-b.y) > epsilon )
	 printf( "POSE FAIL %s expected pose.y %.3f saw pose.y %.3f\n", str, a.y, b.y );  
  if( fabs(a.z-b.z) > epsilon )
	 printf( "POSE FAIL %s expected pose.z %.3f saw pose.z %.3f\n", str, a.z, b.z );  
  if( fabs(a.a-b.a) > epsilon )
	 printf( "POSE FAIL %s expected pose.a %.3f saw pose.a %.3f\n", str, a.a, b.a );  
}

int main( int argc,  char* argv[] )
{
  Init( &argc, &argv);

  StgWorldGui world( 400,400, "Test" );

  world.SetRealTimeInterval( 10000 );

  world.Start();


  stg_geom_t geom;
  bzero( &geom, sizeof(geom) );

  if( 1 )
  {
	 StgModel mod( &world, NULL );
	 
	 // returned pose must be the same as the set pose
	 stg_pose_t pose;
      
  for( stg_meters_t x=0; x<5; x+=0.1 )	 
	 {
		pose = new_pose( x, 0, 0, 0 );
		mod.SetPose( pose );  		
		test( "translate X",  mod.GetPose(), pose );		
		interact( &world );
	 }

  for( stg_meters_t y=0; y<5; y+=0.1 )	 
	 {
		pose = new_pose( 0, y, 0, 0 );
		mod.SetPose( pose );  		
		test( "translate Y",  mod.GetPose(), pose );		
		interact( &world );
	 }

  for( stg_meters_t z=0; z<5; z+=0.1 )	 
	 {
		pose = new_pose( 0, 0, z, 0 );
		mod.SetPose( pose );  		
		test( "translate Z",  mod.GetPose(), pose );		
		interact( &world );
	 }
  
  for( stg_radians_t a=0; a<dtor(360); a+=dtor(2) )	 
	 {
		pose = new_pose( 0, 0, 0, a );
		mod.SetPose( pose );  		  
		pose = mod.GetPose();		
		test( "rotate",  mod.GetPose(), pose );
		interact( &world );
	 }

  for( stg_radians_t a=0; a<dtor(360); a+=dtor(2) )	 
	 {
		pose = new_pose( cos(a), sin(a), 0, 0 );
		mod.SetPose( pose );  		
		test( "translate X&Y",  mod.GetPose(), pose );		
		interact( &world );
	 }

  mod.SetPose( new_pose( 0,0,0,0 ));  		


  for( stg_meters_t x=0.01; x<5; x+=0.1 )	 
	 {
		geom.size.x = x;
		geom.size.y = 1.0;
		geom.size.z = 1.0;
		
		mod.SetGeom( geom );
		interact( &world );
	 }
  
  for( stg_meters_t y=0.01; y<5; y+=0.1 )	 
	 {
		geom.size.x = 5;
		geom.size.y = y;
		geom.size.z = 1.0;
		
		mod.SetGeom( geom );
		interact( &world );
	 }

  for( stg_meters_t z=0.01; z<5; z+=0.1 )	 
	 {
		geom.size.x = 1;
		geom.size.y = 1;
		geom.size.z = z;
		
		mod.SetGeom( geom );
		interact( &world );
	 }
  
  geom.size.x = 0.5;
  geom.size.y = 0.5;
  geom.size.z = 0.5;
  
  mod.SetGeom( geom );
  
  } // mod goes out of scope
  

 #define POP 10
  stg_velocity_t v = {0,0,0,0};
		  
  StgModel* m[POP]; 
  for( int i=0; i<POP; i++ )
	 {
		m[i] = new StgModelLaser( &world, NULL );

		//m[i]->Say( "Hello" );
		m[i]->Subscribe();
		//m[i]->SetGeom( geom );
		
		//m[i]->SetPose( random_pose( -5,5, -5,5 ) );		
		m[i]->PlaceInFreeSpace( 0, 10, 0, 10 );
		m[i]->SetColor( lrand48() | 0xFF000000 );

		v.x = drand48() / 10.0;
		v.y = drand48() / 10.0;
		v.z = drand48() / 10.0;
		v.a = drand48() / 3.0;

		m[i]->SetVelocity( v );
	 }


 //  stg_pose_t o = { 1, 2, 0, 0 };
//   stg_pose_t p = {0,0,0,0};

//   m[0]->SetPose( o );

//   stg_pose_t q = m[0]->LocalToGlobal( p );

//   stg_print_pose( &o );
//   stg_print_pose( &q );

//   assert( fabs( q.x - o.x ) < 0.0001 );
//   assert( fabs( q.y - o.y ) < 0.0001 );
//   assert( fabs( q.z - o.z ) < 0.0001 );
//   assert( fabs( q.a - o.a ) < 0.0001 );

//   m[1]->SetPose( new_pose( 0,0,0,0 ));

  //  geom.size.x = 0.2;
  //geom.size.y = 0.2;
  //geom.size.z = 0.2;

//   for( int i=0; i<POP; i++ )
// 	 {
// 		StgModel* top = new StgModel( &world, m[i] );
// 		top->SetGeom( geom );
// 		//top->SetPose( new_pose( 0,0,0,0 ) );
// 		//m[i]->SetPose( random_pose( -10,10, -10,10 ) );		
// 		//m[i]->PlaceInFreeSpace( -10, 10, -10, 10 );
// 		top->SetColor( lrand48() | 0xFF000000 );
		
//  		//interact( &world );
// 	 }

 //   for( int i=0; i<POP; i++ )
//   	 {
// //  		m[i]->PlaceInFreeSpace( -10, 10, -10, 10 );
//   		//m[i]->SetColor( 0xFF00FF00 );


//  		stg_velocity_t v = {0,0,0,0.1};
		
//  		m[i]->SetVelocity( v );
								
//   		//interact( &world );
//   	 }
  
//   for( int i=0; i<POP; i++ )
//  	 {
//  		delete m[i];
//  		interact( &world );
//  	 }
  

  for( unsigned long i=0; i<1000; i++ )
	 world.Update();

  return 0; // success
}
