#!/bin/bash

#Transfer arguments.
inputFile=$1
inputDirectory=$2

#Temporary storage directory on EOS.
tmpDirectory=/eos/experiment/na61/data/OnlineProduction/2022-p+T2K-OfflineQA/ChunkStorage

#Copy file immediately from CTA into temporary EOS space.
echo '[INFO] Copying file to temporary storage: '$inputDirectory/$inputFile'.'
xrdcp $inputDirectory/$inputFile $tmpDirectory

if [ $? -ne 0 ]
then
  echo "[ERROR] xrdcp of file "$inputDirectory/$inputFile" failed!"
  exit 1
fi

#Move to submit directory and submit QA job.
cd /afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/submit
echo "Submitting file $inputFile in directory $inputDirectory ."
./condor_submit.sh $inputFile $tmpDirectory

exit 0
