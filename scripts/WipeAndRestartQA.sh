#!/bin/bash

if [[ $# -gt 1 ]]
then
    echo "Incorrect usage! Usage: ./WipeAndRestartQA.sh"
    exit 1
fi

echo "Removing QA ROOT files, HTCondor logs, and all running jobs. "
echo "Are you sure you want to proceed? Ctrl-c to abort."

read  -n 1 -p "[press any key to proceed]" input

CleanROOTFiles.sh
CleanLogs.sh
condor_rm na61qa

echo "===================Finished==================="
