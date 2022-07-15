#!/usr/bin/bash

SHINE_SRCS_DIR=/afs/cern.ch/user/n/na61qa/SHINEInstallations
SHINE_VERSION=Master
#SHINE_VERSION=DRS4DataProcessing

source $SHINE_SRCS_DIR/src-$SHINE_VERSION/Tools/Scripts/env/lxplus.sh x86_64-centos7-gcc8-opt
eval $($SHINE_SRCS_DIR/install-$SHINE_VERSION/bin/shine-offline-config --env-sh)
