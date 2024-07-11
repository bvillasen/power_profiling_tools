#!/bin/bash
#SBATCH -J power
#SBATCH --time=1:00:00
#SBATCH --no-requeue
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH -D /lustre/user/bvillase/benchmarks/cholla/hydro/1024_n8
#SBATCH -o run_output.log
#SBATCH -e run_error.log
#SBATCH --exclusive
#SBATCH --exclude x1000c5s4b0n0,x1000c5s6b0n0

echo "Starting lurm job."


pwd; hostname; date
echo "SLUM_NODES=$SLURM_NNODES LIST:$SLURM_NODELIST"

WORK_DIR=$(pwd)
echo "WORK_DIR: $WORK_DIR"

# Collect power profile
export POWER_PROFILE_PATH=$HOME/code/power_profiling_tools/power_profiler
export POWER_PROFILE_OUTPUT_DIR=/lustre/user/bvillase/benchmarks/cholla/hydro/1024_n8
export POWER_PROFILE_MAX_TIME=500
export POWER_PROFILE_SAMPLING_FREQUENCY=100
export POWER_PROFILE_CORE=1
echo "Starting power profile. $(date)"
OUTPUT_FILE=${POWER_PROFILE_OUTPUT_DIR}/power_profile.txt
taskset -c $POWER_PROFILE_CORE $POWER_PROFILE_PATH/power_measure ${OUTPUT_FILE} ${POWER_PROFILE_MAX_TIME} ${POWER_PROFILE_SAMPLING_FREQUENCY} &


echo "Starting application. $(date)"
source $HOME/code/cholla/builds/setup.lockhart.cce.sh
EXE=$HOME/code/cholla/bin/cholla.hydro.lockhart
export OMP_NUM_THREADS=1
export HSA_ENABLE_SDMA=0
srun -u -n 8 --ntasks-per-node=8 --gpus-per-node=8 --cpu-bind=verbose \
        --cpu-bind=mask_cpu:ff000000000000,ff00000000000000,ff0000,ff000000,ff,ff00,ff00000000,ff0000000000  \
        $EXE parameter_file.txt > simulation_output.log

echo "Finished job. $(date)"

# Stop the power profile banckground processes right after the app finishes
bash $POWER_PROFILE_PATH/stop_process.sh 

echo "Exiting slurm job. $(date)"