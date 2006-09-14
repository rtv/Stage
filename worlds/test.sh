#!/bin/bash

#CONT=/Users/vaughan/PS/HEAD/player/examples/c++/laserobstacleavoid
#CONT=$HOME/PS/HEAD/player/examples/c++/laserobstacleavoid

for (( PORT=6665 ; $PORT < (6665 + $1) ; PORT++ )) ; do

  echo $PORT
  $2 -p $PORT > /dev/null & 
  sleep 1

done
  