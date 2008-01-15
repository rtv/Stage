#!/bin/bash

X=10
Y=20

for (( INDEX=0 ; $INDEX < $1 ; INDEX++ )) ; do
  #X=$(($RANDOM % 100))
  #Y=$(($RANDOM % 100))
  A=$(($RANDOM % 360))

  #P=$(bc <<< "scale=3; ($X * ( $2 / 32767.0 )) + $4")
  #Q=$(bc <<< "scale=3; ($Y * ( $3 / 32767.0 )) + $5")

 # P=$(bc <<< "scale=3; ($X / 4.0 ) + $4")
 # Q=$(bc <<< "scale=3; ($Y / 4.0 ) + $5")

  

  echo "swarmbot( name \"r$INDEX\" pose [ -18 0 $A ] )" 
done
  