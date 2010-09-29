#!/bin/bash

#CONT=$HOME/PS/HEAD/player/examples/c++/laserobstacleavoid

# echo $1

for (( INDEX=0 ; $INDEX < $1 ; INDEX++ )) ; do

  PORT=$(($INDEX+6665))  

  echo "driver( name \"stage\" provides [\"$PORT:position:0\" \"$PORT:laser:0\" \"$PORT:sonar:0\" \"$PORT:blobfinder:0\"]  model \"robot$INDEX\" )"

done
  