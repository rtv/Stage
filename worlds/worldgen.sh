#!/bin/bash

I=0
for (( X=0 ; $X < $1 ; X++ )) ; do

   P=$(bc <<< "scale=3; ($X / 4.0 ) + $3")

  for (( Y=0 ; $Y < $2 ; Y++ )) ; do
    #X=$(($RANDOM % 100))
    #Y=$(($RANDOM % 100))
    #A=$(($RANDOM % 360))

    A=0
    Q=$(bc <<< "scale=3; ($Y / 4.0 ) + $4")


    #echo "swarmbot( name \"r$X\" pose [ -18 0 $A ] )" 
    echo "swarmbot( name \"r$I\" pose [ $P $Q $A ] )" 

    I=$(($I+1))
  done
done