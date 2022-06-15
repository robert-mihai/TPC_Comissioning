#!/bin/bash
#Code to separate folders into folders with at most 5000 events. 

name_array=()
for dir in *_ii_*/; do
 temp=$(echo $dir)
 name_array+=${temp%?}
for base_name in $name_array; do
 a_name=$base_name"_a"
 b_name=$base_name"_b"
 rm -r $a_name
 rm -r $b_name
 mkdir $a_name
 mkdir $b_name
 cp $base_name"/C1Test0000.txt" $a_name"/C1Test0000.txt"
 cp $base_name"/C1Test0000.txt" $b_name"/C1Test0000.txt"
 for i in {0..5000}; do 
  filename="/C4Test000""$i"".txt"
  cp $base_name$filename $a_name$filename
 done
 for i in {5001..10000}; do
  filename="/C4Test000""$i"".txt"
  cp $base_name$filename $b_name$filename
 done
done
done
