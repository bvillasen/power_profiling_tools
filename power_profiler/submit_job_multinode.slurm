#!/bin/bash
#SBATCH -J power
#SBATCH --time=1:00:00
#SBATCH --no-requeue
#SBATCH --nodes=4
#SBATCH --cpus-per-task=1
#SBATCH -D /lustre/user/bvillase/benchmarks/cholla/cosmology/1024_50Mpc
#SBATCH -o run_output.log
#SBATCH -e run_error.log
#SBATCH --exclusive
#SBATCH -w x1000c0s4b0n0,x1000c0s4b1n0,x1000c2s1b0n0,x1000c2s1b1n0
##SBATCH -w x1000c6s1b1n0,x1000c4s4b1n0,x1000c6s1b1n0,x1000c1s1b1n0
#SBATCH --exclude x1000c5s4b0n0,x1000c5s6b0n0

echo "Starting lurm job."


pwd; hostname; date
echo "SLUM_NODES=$SLURM_NNODES LIST:$SLURM_NODELIST"

WORK_DIR=$(pwd)
echo "WORK_DIR: $WORK_DIR"

# Collect power profile
export POWER_PROFILE_PATH=$HOME/code/power_profiling_tools/power_profiler
export POWER_PROFILE_OUTPUT_DIR=/lustre/user/bvillase/benchmarks/cholla/cosmology/1024_50Mpc/power
export POWER_PROFILE_MAX_TIME=300
export POWER_PROFILE_SAMPLING_FREQUENCY=100
export POWER_PROFILE_CORE=1
echo "Starting power profile. $(date)"
srun -n $SLURM_NNODES --ntasks-per-node=1 --gpus-per-node=8 bash $POWER_PROFILE_PATH/run_in_background.sh 

echo "Starting job. $(date)"
source $HOME/code/cholla_cosmo/builds/setup.lockhart.cce.sh
EXE=$HOME/code/cholla_cosmo/bin/cholla.cosmology.lockhart
export OMP_NUM_THREADS=1
export HSA_ENABLE_SDMA=0
srun -u -n 32 --ntasks-per-node=8 --gpus-per-node=8 --cpu-bind=verbose \
        --cpu-bind=mask_cpu:ff000000000000,ff00000000000000,ff0000,ff000000,ff,ff00,ff00000000,ff0000000000  \
        $EXE parameter_file.txt > simulation_output.log

echo "Finished job. $(date)"

# Stop the power profile banckground processes
srun -n $SLURM_NNODES --ntasks-per-node=1 --gpus-per-node=8 bash $POWER_PROFILE_PATH/stop_process.sh 

echo "Exiting slurm job. $(date)"