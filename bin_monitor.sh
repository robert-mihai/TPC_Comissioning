#!/bin/bash
oldsz=""
for (( ; ; ))
do
  newstring=$(ls -lh ""$1"/binary_dat")
  arr=($newstring)
  newsz="${arr[4]}"
  if [ ! "$newsz" = "$oldsz" ]; then
      echo "$newstring"
      oldsz="$newsz"
  fi
done

