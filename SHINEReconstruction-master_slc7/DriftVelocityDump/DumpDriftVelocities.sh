#!/bin/bash

#Tool for dumping timestamped nominal drift velocities to text file in
#order to reconstruct approximately correct cluster y-positions. From
#the mind of Sir Tobiasz of DCS.

#channels:
#  145 | cm/us  | MTPC-R drift velocity
#  146 | cm/us  | MTPC-L drift velocity
#  147 | cm/us  | VTPC-1 drift velocity
#  148 | cm/us  | VTPC-2 drift velocity
#  149 | cm/us  | GapTPC drift velocity
#  202 | cm/us  | FTPC1
#  192 | cm/us  | FTPC1
#  193 | cm/us  | FTPC1

#Assign start & stop periods to define dump edges.
START_DATE='2022-06-19'
START_TIME='00:00:00'

CURRENT_DATE=$(date +"%Y-%m-%d")
CURRENT_TIME=$(date +"%T")


echo 'Dumping drift velocities from '$START_DATE' '$START_TIME' to '$CURRENT_DATE' '$CURRENT_TIME'.'

./DumpDriftVelocity 147 `date +"%s" -d $START_DATE" "$START_TIME` `date +"%s" -d $CURRENT_DATE" "$CURRENT_TIME` > VTPC1.txt
./DumpDriftVelocity 148 `date +"%s" -d $START_DATE" "$START_TIME` `date +"%s" -d $CURRENT_DATE" "$CURRENT_TIME` > VTPC2.txt
./DumpDriftVelocity 145 `date +"%s" -d $START_DATE" "$START_TIME` `date +"%s" -d $CURRENT_DATE" "$CURRENT_TIME` > MTPCL.txt
./DumpDriftVelocity 146 `date +"%s" -d $START_DATE" "$START_TIME` `date +"%s" -d $CURRENT_DATE" "$CURRENT_TIME` > MTPCR.txt
./DumpDriftVelocity 149 `date +"%s" -d $START_DATE" "$START_TIME` `date +"%s" -d $CURRENT_DATE" "$CURRENT_TIME` > GTPC.txt
./DumpDriftVelocity 202 `date +"%s" -d $START_DATE" "$START_TIME` `date +"%s" -d $CURRENT_DATE" "$CURRENT_TIME` > FTPC1.txt
./DumpDriftVelocity 192 `date +"%s" -d $START_DATE" "$START_TIME` `date +"%s" -d $CURRENT_DATE" "$CURRENT_TIME` > FTPC2.txt
./DumpDriftVelocity 193 `date +"%s" -d $START_DATE" "$START_TIME` `date +"%s" -d $CURRENT_DATE" "$CURRENT_TIME` > FTPC3.txt
