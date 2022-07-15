#!/bin/bash

#############################################################
echo ""
echo "========================= START ========================="

#To run: "source ./condor_submit listOfDataFiles.txt",
#where each line of file list is the full path to the file

# AMOUNT OF CLUSTERS THIS JOB WILL RUN ON
NUM_CLUSTERS=1

#Place of work dir. Use afs, as eos has IO problems
RECO_WORK_DIR=/afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/work

#Log output directory. Use afs, not eos for logs
LOG_DROP_DIR=/afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/log.bz2

#Directory of reconstruction sequence. This should probably be afs as well
SRC_DIR=/afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/SHINEReconstruction-master_slc7

#Place of output shoe.root files. CTA directory needs to already be made
RECO_DROP_DIR_EOS=/eos/experiment/na61/data/OnlineProduction/2022-p+T2K-OfflineQA

CONDOR_CMD=condor_submit
CMD=condor_run.sh

#For reconstruction jobs.
QUEUE=workday

# espresso     = 20 minutes
# microcentury = 1 hour
# longlunch    = 2 hours
# workday      = 8 hours
# tomorrow     = 1 day
# testmatch    = 3 days
# nextweek     = 1 week

inputFile=$1
inputDirectory=$2

#Job batch name
jobBatchName="QA-"$inputFile

echo 'Submitting QA reconstruction for input file '$inputFile' in directory '$inputDirectory

#############################################################

timestamp=$(date +%Y%m%d_%H%M%S)

work_dir=$RECO_WORK_DIR/$timestamp
mkdir -p "$work_dir"
echo "[INFO] Work directory: $work_dir"

batch_dir=$work_dir"/batch"
mkdir -p $batch_dir

echo "[INFO] Batch directory: $batch_dir"

drop_dir_1=$RECO_DROP_DIR_EOS
mkdir -p $drop_dir_1
echo "[INFO] Drop directory EOS: $drop_dir_1"

log_drop_dir=$LOG_DROP_DIR
mkdir -p $log_drop_dir
echo "[INFO] Log Drop directory: $log_drop_dir"

rsync -r ./condor_run.sh   $batch_dir
rsync -r ./condor.sub.in.head $batch_dir
rsync -r --exclude=*root --exclude=*raw  $SRC_DIR/ $batch_dir/Prod

chmod -R 0755 $batch_dir

##############################
#	INSIDE BATCH
#############################
cd $batch_dir

tar --remove-files -zcf Stage.tar.gz ./Prod

transfer_input_files=$batch_dir/Stage.tar.gz

#
# CLEAN-UP BATCH
#
rm -f ./*.sub

#
# POPULATE HEAD OF SUB
#

execution_prefix=`head /dev/urandom | tr -dc A-Za-z0-9 | head -c8`

sub_head_name=.${execution_prefix}_condor.sub.head
cat ./condor.sub.in.head | sed \
	-e "s~@EXECUTABLE@~${batch_dir}/${CMD}~g" \
	-e "s~@QUEUE@~${QUEUE}~g" \
	-e "s~@TRANSFER_INPUT_FILES@~${transfer_input_files}~g" \
	-e "s~@TRANSFER_OUTPUT_FILES@~${transfer_output_files}~g" \
> $sub_head_name

#This section creates a subdirectory for each chunk name within the work directory. 
#If it doesn't, there's a bash scripting problem! Check submission output for the
#echo ...----------------short_inputfile----------------... line.
sub_batch_name=.${execution_prefix}_condor.sub

if ! [[ -e $sub_batch_name ]]; then cat $sub_head_name > $sub_batch_name; fi 

short_inputfile=${inputFile%%.*}
echo "--------------------"$short_inputfile"--------------------"

initial_dir=$work_dir/$short_inputfile
mkdir -p $initial_dir
echo "[INFO] InitialDir $initial_dir"

condor=$initial_dir/condor_${short_inputfile}.sub

cat $sub_head_name > $condor

echo | tee -a $condor $sub_batch_name
echo "initialdir  = $initial_dir"				   | tee -a $condor $sub_batch_name > /dev/null
echo "arguments   = $inputFile $inputDirectory $drop_dir_1 $log_drop_dir"       | tee -a $condor $sub_batch_name > /dev/null
echo "queue"                                                     | tee -a $condor $sub_batch_name > /dev/null

#
# SUBMIT TASK
#
echo Submitting jobs...
echo

counter=0

for sub in `find ./ -type f -name *.sub`
do
	while [ $counter -lt 5 ]
	do
	    echo Submitting ${sub}... try $counter
	    $CONDOR_CMD -batch-name $jobBatchName  $sub
            if [ $? -eq 0 ]
	    then
                break
            fi
            sleep 30
            let counter=counter+1
        done
done

echo Cleaning up...
echo rm -f .${execution_prefix}*
rm -r .${execution_prefix}*

echo "------------------------------------------------"
cd -

#############################################################

echo "========================= END ========================="
echo ""
