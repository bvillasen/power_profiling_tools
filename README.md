# Power Profiling Tools
Tools to measure GPU power usage


## Power Profiler

Simple script to collect power measurements at a given sampling frequency

To build 
```
module load rocm
cd power_profiler
make
```

This builds the *power_measure* binary that can be run by passing the output_file where the power measurements will be written, the maximum time that the program will collect power in seconds, and the sampling frequency in Hz. 

To run
```
./power_measure output_file time_max sampling_frequency
```

A SLURM scripts with an example to run the power profiler with an application here: *power_profiler/submit_job_simglenode.slurm*  


## Plotting the Profiles

In the *plotting_tools* directory, there is a simple python script to plot the power profile.   