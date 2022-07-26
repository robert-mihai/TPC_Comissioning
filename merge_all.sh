#!/bin/bash

#SBATCH --job-name merge_num
#SBATCH --error Log/merge_num-error.e%A_%a
#SBATCH --output Log/merge_num-out.o%A_%a
#SBATCH --ntasks 1
#SBATCH --cpus-per-task 1
#SBATCH --partition public-cpu
#SBATCH --time 0-02:30:00

module load GCCcore/8.3.0
srun merge_num.sh "${SLURM_ARRAY_TASK_ID}" | tee Log/merge_num-stdout.txt