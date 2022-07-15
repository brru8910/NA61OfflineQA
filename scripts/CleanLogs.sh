#!/bin/bash

if [[ $# -gt 2 ]]
then
    echo "Incorrect usage! Usage: ./CleanLogs.sh [optional flag -i for prompting before deletion]"
    exit 1
fi

xargsInteractiveFlag=''
if [[ $# -eq 1 ]]
then
    xargsInteractiveFlag='--interactive'
fi


#Number of logs to preserve (not delete). Some amount of Condor jobs
#will be continuously running while data taking is ongoing, and logs
#_must not_ be delted while jobs are running. This number is a buffer
#to prevent any currently-running job logs from being removed.

#If you are uncertain about how many logs to preserve, check number of
#running jobs with 'condor_q'for an idea and give yourself plenty of
#overhead.
nLogsToKeep=500

#Clean out Condor logs (uncompressed). Use find, which does not have
#the result list limitations of ls (or rm). Pipe output to head,
#preserve nLogsToKeep, and pipe to xargs-wrapped rm.

find /afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/work/ -maxdepth 1 -name '*' | tail -n +2 | head -n -$nLogsToKeep | xargs $xargsInteractiveFlag rm -r

#Clean out .bz2 logs using same technique.
find /afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/log.bz2/ -maxdepth 1 -name '*' |  tail -n +2  | head -n -$nLogsToKeep | xargs $xargsInteractiveFlag rm -r
