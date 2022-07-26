#!/bin/bash
foldername=$( sed "${1}q;d" filenames.txt )

binary_file_name="binary_dat"
num_files_per_folder=5000
executable_name="merge" #this executable must be in the same directory as this script is being run from

#convertToString function thanks to dogbane on stackexchange
alphabet=( {a..z} )
convertToString(){
    dividend="$1"
    colName=""
    while (( dividend > 0 ))
    do
         mod=$(( (dividend-1)%26 ))
         colName="${alphabet[$mod]}$colName"
         dividend=$(( (dividend-mod)/26 ))
    done
    echo "$colName"
}

 if ! [ "_" == ${foldername:-3:1} ]; then #We want to exclude _a, _b, _c, etc. partition folders produced in previous runs. Technically if we have crazy numbers of folders this will no longer work, i.e. if we get to _aa, _ab, _ac, etc., but this is better than nothing. 
    #we will split into new folders to only have num_files_per_folder C4Tests in each folder
    num_files=$(find $foldername"/C4Test000"*".txt" -maxdepth 1 -type f | wc -l) 
    num_new_folders=$(( (num_files+num_files_per_folder-1)/num_files_per_folder ))
    if (( $num_new_folders > 1 )); then
        for foldnum in $( seq 1 $num_new_folders ); do 
            #give each new folder a unique name by adding _a, _b, etc. to the end of the name of the original folder
            letter="$(convertToString "$foldnum")"
            name="$foldername""_""$letter"
            if ! [ -s $name"/"$binary_file_name ]; then
                rm -r $name #clear any old folder that was there
                mkdir $name #make a new folder to hold this set of data
                #copy files over to new folders
                cp $foldername"/C1Test0000.txt" $name"/C1Test0000.txt"
                first_idx=$(( 5000 * ($foldnum - 1) ))
                    last_idx=$(( $first_idx + 4999 ))
                    ctr=0
                    for i in $( seq $first_idx $last_idx ); do 
                    filename="/C4Test000""$i"".txt"
                    copyname="/C4Test000""$ctr"".txt"
                    cp "$foldername""$filename" "$name""$copyname"
                    ctr=$(( $ctr + 1 ))
                done 
                ./$executable_name $name
                #Remove the old files, as they are no longer needed. Comment these two lines out if you want to keep them. 
                #rm "$foldername"/C?Test000*.txt
                #rm "$name"/C?Test000*.txt
            fi
        done 
    elif (( $num_new_folders == 1 )); then
        if ! [ -s $foldername"/"$binary_file_name ]; then
            ./$executable_name $foldername
            #Remove the old files, as they are no longer needed. Comment this line out if you want to keep them. 
            #rm "$foldername"/C?Test000*.txt
        fi
    fi
fi