

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define USAGE \
"USAG: make_motes [-n <number> -p <base port> -f <pnm file>"\
"                  -r <resolution> -s <scale> -i <base index>"\
"                  -o <world_file> -g <swarm geometry>]"

/************************************************************************/

typedef struct{
  double x;
  double y;
  double th;
} pos_t;

pos_t *pos_list;

int num_nodes = 25;
int base_index = 0;
int base_port = 6660;
float scale = 0.09;
float res   = 0.15;
char shape_name[32] = "random";
char world_filename[32] = "gts.world";
char pnm_filename[32] = "simple.pnm";

FILE *world_file;

/************************************************************************/

void parse_args(int argc, char** argv)
{
  int i;

  i=1;
  while(i<argc){

    if(!strcmp(argv[i],"-g")){
      if(++i<argc){
	strcpy(shape_name,argv[i]);
      }
      else{
        puts(USAGE);
        exit(1);
      }
    }
    
    else if(!strcmp(argv[i],"-o")){
      if(++i<argc)
	strcpy(world_filename,argv[i]);
      else{
        puts(USAGE);
        exit(1);
      }
    }

    else if(!strcmp(argv[i],"-n")){
      if(++i<argc){
        num_nodes = atoi(argv[i]);
      }
      else{
        puts(USAGE);
        exit(1);
      }
    }

    else if(!strcmp(argv[i],"-i")){
      if(++i<argc){
        base_index = atoi(argv[i]);
      }
      else{
        puts(USAGE);
        exit(1);
      }
    }
    
    else if(!strcmp(argv[i],"-s")){
      if(++i<argc){	
        scale = atof(argv[i]);
      }
      else{
        puts(USAGE);
        exit(1);
      }
    }

    else if(!strcmp(argv[i],"-r")){
      if(++i<argc){
        res = atof(argv[i]);
      }
      else{
        puts(USAGE);
        exit(1);
      }
    }    

    else if(!strcmp(argv[i],"-p")){
      if(++i<argc)
        base_port = atoi(argv[i]);
      else{
        puts(USAGE);
        exit(1);
      }
    }
    else{
      puts(USAGE);
      exit(1);
    }
    i++;
  }
}

/************************************************************************/

#define X_BASE_OFFSET 15.0
#define Y_BASE_OFFSET 15.0
#define POS_SCALER    3.0
#define DEG_SCALER    3.0

pos_t *make_wave(void)
{
  int i, j, sq_side;
  sq_side =(int)sqrt((double)num_nodes); 
  
  for(j = 0 ; j < sq_side; j++){
    for(i = 0 ; i < sq_side; i++){
      pos_list[j*sq_side+i].x  = i*POS_SCALER + X_BASE_OFFSET;
      pos_list[j*sq_side+i].y  = j*POS_SCALER + Y_BASE_OFFSET; 
      pos_list[j*sq_side+i].th = (j*sq_side + i) * DEG_SCALER; 
    }
  }

  return NULL;
}

/************************************************************************/

pos_t *make_random(void)
{
  int i, j, sq_side;
  sq_side =(int)sqrt((double)num_nodes); 
  
  srand(time(NULL));
  
  for(j = 0 ; j < sq_side; j++){
    for(i = 0 ; i < sq_side; i++){
      pos_list[j*sq_side+i].x  = i*POS_SCALER + X_BASE_OFFSET;
      pos_list[j*sq_side+i].y  = j*POS_SCALER + Y_BASE_OFFSET; 
      pos_list[j*sq_side+i].th = rand()%360; 
    }
  }
  
  return NULL;
}

/************************************************************************/

pos_t *make_grid(void)
{
  int i, j, sq_side;
  sq_side =(int)sqrt((double)num_nodes); 
  
  for(j = 0 ; j < sq_side; j++){
    for(i = 0 ; i < sq_side; i++){
      pos_list[j*sq_side+i].x  = i*POS_SCALER + X_BASE_OFFSET;
      pos_list[j*sq_side+i].y  = j*POS_SCALER + Y_BASE_OFFSET; 
      pos_list[j*sq_side+i].th = 15; 
    }
  }
  
  return NULL;
}

/************************************************************************/

pos_t *build_pos_list(void)
{
  pos_list = (pos_t*)malloc(sizeof(pos_t) * num_nodes);
  
  if(!strcmp(shape_name, "random")) return make_random();
  if(!strcmp(shape_name, "wave")) return make_wave();
  if(!strcmp(shape_name, "grid")) return make_grid();
  printf("Unknown shape: %s!\n", shape_name);
  exit(3);
}

/************************************************************************/

int main(int argc, char** argv)
{
  int i; 

  parse_args(argc, argv);

  if((world_file = fopen(world_filename, "w")) == NULL){
    perror("opening world file");
    exit(1);
  }
  
  build_pos_list();

  fprintf(world_file, "begin environment\n"
                      "  file %s\n"
	              "  resolution %.3f\n"
		      "  scale %.3f\n"
                      "end\n\n",
	  pnm_filename, res, scale);
  
  for(i = 0 ; i < num_nodes; i++){
    fprintf(world_file, "begin position_device\n"
	    "  port %d\n"
            "  pose (%.1f %.1f %.1f)\n"
	    "  begin sonar_device\n"	
	    "  end\n"	
	    "  begin mote_device\n"
	    "    index %d\n"
	    "  end\n"	
	    "  begin player_device\n"
	    "  end\n"
	    "end\n\n",
	    i+base_port, pos_list[i].x, pos_list[i].y, 
            pos_list[i].th, i+base_index);
  }

  printf("wrote world file: %s\n"
	 "           nodes: %d\n"
	 "           shape: %s\n"
	 "       base port: %d\n"
	 "      base index: %d\n"
	 "       .pnm file: %s\n"
	 "      resolution: %.3f\n"
         "           scale: %.3f\n",
	 world_filename, num_nodes, shape_name, 
	 base_port, base_index, pnm_filename, res, scale);
}
