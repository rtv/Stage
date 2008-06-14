#!/bin/bash

for (( INDEX=0 ; $INDEX < $1 ; INDEX++ )) ; do
  X=$(($RANDOM % $2))
  Y=$(($RANDOM % $3))
  A=$(($RANDOM % 360))
  X=$(($X - $2 / 2))
  Y=$(($Y - $3 / 2))
  echo "pioneer2dx( name \"robot$INDEX\" pose [$X $Y $A] laser() blobfinder() )"
done
  