#!/bin/bash

X=10
Y=20

for (( INDEX=0 ; $INDEX < $1 ; INDEX++ )) ; do
  X=$(($RANDOM)) 
  Y=$(($RANDOM))
  A=$(($RANDOM % 360))

  #P=$(bc <<< "scale=4; ($X * ( $2 / 32767.0 )) - $2/2.0")
  #Q=$(bc <<< "scale=4; ($Y * ( $3 / 32767.0 )) - $3/2.0")
  P=$(bc <<< "scale=4; ($X * ( $2 / 32767.0 )) + $4")
  Q=$(bc <<< "scale=4; ($Y * ( $3 / 32767.0 )) + $5")

  echo "robot( name \"r$INDEX\" pose [ $P $Q $A ] )" 
done
  