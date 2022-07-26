July 26th, 2022, Edward John Roe
Notes on how to use the programs in this folder: 
The following three files in this folder: 
merge_binary_3.cpp
merge_num.sh
merge_all.sh
are used to create the binary data files for the data from the Lecroy PMT configuration of the TPC. 
To use them on baobab, do the following: 
1. Load the gcc core module by running
module load GCCcore/8.3.0
2. Compile the merge program by running
g++ -O3 merge_binary_3.cpp -o merge
The -O3 option helps the binary to run faster, which is important because it takes a long time. The -o merge makes the name of the executable be "merge." This can be changed but you will also need to edit the merge.sh script to reflect this. 
3. Copy merge,  merge_num.sh and merge_all.sh to the directory you want to work in, e.g. Config_18. 
4. cd into that directory, and make sure you have set permissions using chmod for the programs to run. I usually set permissions for these purposes at chmod 750. 
5. We now need to set up some things in the folder. First, remove any old clutter you do not need (this is optional but will probably make your life easier). Then make a text file containing the name of each folder you need to make the binary for as a separate line. The easiest way to do this, assuming that your folder is not crazily cluttered, is to run ls > filenames.txt, and then go in and manually remove the irrelevant files. The name of this file is specified in merge_num.sh so should remain the same unless you change it there as well. 
Next, make a directory called Log. This is where the error/out files will be stored if needed. 
6. Start a slurm array batch job. First you will need to know how many jobs you are actually doing. The easiest way is to use your text file from before: just type wc -l | filenames.txt, and the number is how many files you have. Then run
sbatch --array=1-number_of_files%100 merge_all.sh
where number_of_files is the number you got above. You can also do this as a one-liner if you want.The %100 limits it to run no more than 100 jobs at once; this is a courtesy to other users. 
7. To check the status of your jobs, you can run squeue -u $USER. 
8. For more help on slurm, you can visit https://doc.eresearch.unige.ch/hpc/slurm. Slurm also has the ir own website, which is very helpful: https://slurm.schedmd.com/. 