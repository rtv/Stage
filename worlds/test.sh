#!/bin/bash

#CONT=/Users/vaughan/PS/HEAD/player/examples/c++/laserobstacleavoid
CONT=/Users/vaughan/PS/HEAD/player/examples/c++/sonarobstacleavoid

# echo $1

for (( PORT=6665 ; $PORT < (6665 + $1) ; PORT++ )) ; do

  echo $PORT
  $CONT -p $PORT > /dev/null & 

done
  