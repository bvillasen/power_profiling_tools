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

This builds the power_measure binary that can be run by passing the output_file to write the power measurements, the maximum time that the program will collect power in seconds, and the sampling frequency in Hz. 

To run
```
./power_measure output_file time_max sampling_frequency
```