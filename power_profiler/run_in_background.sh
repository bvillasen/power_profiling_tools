#!/bin/bash

if [ ! -d "$POWER_PROFILE_OUTPUT_DIR" ]; then
    mkdir -p $POWER_PROFILE_OUTPUT_DIR
fi
OUTPUT_FILE=${POWER_PROFILE_OUTPUT_DIR}/power_profile_${SLURM_NODEID}.txt
taskset -c $POWER_PROFILE_CORE $POWER_PROFILE_PATH/power_measure ${OUTPUT_FILE} ${POWER_PROFILE_MAX_TIME} ${POWER_PROFILE_SAMPLING_FREQUENCY} &


# echo -ne '\n' | taskset -c $POWER_PROFILE_CORE $POWER_PROFILE_PATH/power_measure ${OUTPUT_FILE} ${POWER_PROFILE_MAX_TIME} ${POWER_PROFILE_SAMPLING_FREQUENCY} &

