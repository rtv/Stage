/*
 * $Id: autoplace.cc,v 1.1 2002-10-16 20:49:08 gerkey Exp $
 *
 * util to autoplace robots (or other entities with position and truth devices)
 * randomly, in free space, inside a given rectangle
 *
 */

#include <stdio.h>
#include <string.h> /* for strcpy() */
#include <stdlib.h>  /* for atoi(3),rand(3) */
#include <time.h>  /* for time(2) */
#include <math.h>  /* for M_PI */

#include <playerclient.h>

#define USAGE \
  "USAGE: autoplace [-h <host>] [-p <port>] <bots> <x0> <y0> <x1> <y1>\n" \
  "       -h <host> : connect to Player on this host\n" \
  "       -p <port> : start with this base port\n" \
  "       <bots>    : number of robots to place\n" \
  "       <x0>      : x-coord of lower left corner of rectangle (meters)\n" \
  "       <y0>      : y-coord of lower left corner of rectangle (meters)\n" \
  "       <x1>      : x-coord of upper right corner of rectangle (meters)\n" \
  "       <y1>      : y-coord of upper right corner of rectangle (meters)\n"

char host[256] = "localhost";
int baseport = PLAYER_PORTNUM;
int bots;
int myx0,myy0,myx1,myy1;

/* parse command-line args */
int
parse_args(int argc, char** argv)
{
  int i;

  if(argc < 6)
    return(-1);

  i=1;
  while(i<argc-5)
  {
    if(!strcmp(argv[i],"-h"))
    {
      if(++i<argc)
        strcpy(host,argv[i]);
      else
        return(-1);
    }
    else if(!strcmp(argv[i],"-p"))
    {
      if(++i<argc)
        baseport = atoi(argv[i]);
      else
        return(-1);
    }
    else
      return(-1);
    i++;
  }

  bots = atoi(argv[i++]);
  myx0 = atoi(argv[i++]);
  myy0 = atoi(argv[i++]);
  myx1 = atoi(argv[i++]);
  myy1 = atoi(argv[i]);

  return(0);
}

int
main(int argc, char** argv)
{
  PlayerClient robot;
  double randx,randy,randth;
  double x,y,th;

  srand(time(NULL));

  if(parse_args(argc,argv))
  {
    puts(USAGE);
    exit(-1);
  }

  for(int port=baseport;port<baseport+bots;port++)
  {
    if(robot.Connect(host,port))
      exit(-1);

    PositionProxy pp(&robot,0,'r');
    TruthProxy tp(&robot,0,'a');

    robot.Read();

    for(robot.Read(),tp.GetPose(&x,&y,&th);
        pp.stall || x<myx0 || x>myx1 || y<myy0 || y>myy1;
        robot.Read(),tp.GetPose(&x,&y,&th))
    {
      randx = ((1000*myx0)+(rand() % (1000*(myx1-myx0))))/1000.0;
      randy = ((1000*myy0)+(rand() % (1000*(myy1-myy0))))/1000.0;
      randth = (rand() % 360) * M_PI / 180.0;

      if(tp.SetPose(randx,randy,randth))
        exit(-1);
    }
    robot.Disconnect();
  }

  return(0);
}

