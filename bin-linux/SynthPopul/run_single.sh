#!/bin/bash

% This is the script to generate the synthetic population for one country.

% 1. Note the appropriate population/countries hdr/flt files must exist in the RequiredData directory.
% 2. Change the REPO so it points to a network accessible folder containing the Gloabl Sim directory structure.
% 3. Change CLUSTER_CMD to however you launch a job on your cluster. (Or blank to run locally)
% 4. Run generate_synth_popul.sh

REPO="//fi--didef2/GLOBALSIM/RELEASE"
CLUSTER_CMD="qsub "

SCRIPT_ROOT=$REPO/bin-linux/SynthPopul/scripts
EXE_PATH=$REPO/bin-linux/SynthPopul/SynthPopul
POP_HDR=$REPO/data/RequiredData/population.hdr
POP_FLT=$REPO/data/RequiredData/population.flt
CTR_HDR=$REPO/data/AdminUnits/cbr.hdr
CTR_FLT=$REPO/data/AdminUnits/cbr.flt
POP_OUT=$REPO/data/SyntheticPopulation
DATA_IN=$REPO/data/SyntheticPopulation/params

$CLUSTER_CMD $SCRIPT_ROOT/$1/$2/run__SynthPopul__$2.sh $EXE_PATH $POP_HDR $POP_FLT $CTR_HDR $CTR_FLT $POP_OUT/popul__$2 $POP_OUT/est__$2 $DATA_IN/$1/$2 $SCRIPT_ROOT/$1/$2