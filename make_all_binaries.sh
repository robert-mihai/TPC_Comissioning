#!/bin/bash
#Notes: July 19th, 2022, Edward John Roe
#Code to separate folders into folders with at most num_files_per_folder events, and then make the binary for each folder. 
#when running the script, the user should enter the name of the folder they want to produce binaries for. 
#Example: Suppose we have a directory SWAN_Data on CERNBox and that it contains a subdirectory Configuration_18 which contains subdirectories with the data files C1Test0000.txt and C4Test000x.txt. We then run the following command: 
#user@cernbox.cern.ch/SWAN_Data: ./make_all_binaries.sh Configuration_18
#where Configuration_18 is a folder with subfolders each containing any number of events. 
#Result: All data folders within the Configuration_18 folder are split into subfolders containing no more than num_files_per_folder events each. 
#Alternatively, the script can be run locally from within the Configuration_xx folder by not specifying any option, e.g.
#user@cernbox.cern.ch/SWAN_Data/Configuration_18: ./make_all_binaries.sh
binary_file_name="binary_dat"
num_files_per_folder=5000
executable_name="merge" #this executable must be in the same directory as this script is being run from

initiating_directory=$pwd

str="$1"
if [ $# -eq 0 ]; then
    original_folder_name="../"
else
    if [ "/" == ${str: -1} ]; then
        original_folder_name=$str
    else
        original_folder_name=$str"/"
    fi
    cp $executable_name $original_folder_name
    cd $original_folder_name
fi

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

#First we collect the names of all the directories we want to run through. This helps avoid infinite loops, as the program will create new directories. 
name_array=() 
index=0
for dir in */; do 
 temp=$(echo $dir)
 if ! [ "_" == ${temp: -3:1} ]; then #We want to exclude _a, _b, _c, etc. partition folders produced in previous runs. Technically if we have crazy numbers of folders this will no longer work, i.e. if we get to _aa, _ab, _ac, etc., but this is better than nothing. 
 name_array[index]=${temp%?}
 index=$index+1
 fi
done
for base_name in ${name_array[@]}; do 
    echo "Working on ""$base_name"
    #we will split into new folders to only have num_files_per_folder C4Tests in each folder
    num_files=$(find $base_name"/C4Test000"*".txt" -maxdepth 1 -type f | wc -l) 
    num_new_folders=$(( (num_files / num_files_per_folder) + 1 ))
    if (( $num_new_folders > 1 )); then
        for foldnum in $( seq 1 $num_new_folders ); do 
            #give each new folder a unique name by adding _a, _b, etc. to the end of the name of the original folder
            letter="$(convertToString "$foldnum")"
            name="$base_name""_""$letter"
            if ! [ -s $name"/"$binary_file_name ]; then
                rm -r $name #clear any old folder that was there
                mkdir $name #make a new folder to hold this set of data
                #copy files over to new folders
                cp $base_name"/C1Test0000.txt" $name"/C1Test0000.txt"
                first_idx=$(( 5000 * ($foldnum - 1) ))
                    last_idx=$(( $first_idx + 4999 ))
                    for i in $( seq $first_idx $last_idx ); do 
                    filename="/C4Test000""$i"".txt"
                    cp "$base_name""$filename" "$name""$filename"
                done 
                ./$executable_name $name
                #Remove the old files, as they are no longer needed. Comment these two lines out if you want to keep them. 
                rm "$base_name"/C?Test000*.txt
                rm "$name"/C?Test000*.txt
            fi
        done 
    elif (( $num_new_folders == 1 )); then
        if ! [ -s $base_name"/"$binary_file_name ]; then
            ./$executable_name $base_name
            #Remove the old files, as they are no longer needed. Comment this line out if you want to keep them. 
            rm "$base_name"/C?Test000*.txt
        fi
    fi
done 
cd $initiating_directory #Return to the directory we started from, so as not to annoy the user. 