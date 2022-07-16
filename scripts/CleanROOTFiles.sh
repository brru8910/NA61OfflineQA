#!/bin/bash

#Clean out root files.
find /afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/EOSDropDirectory/ -maxdepth 1 -name "*.root" -delete

#Clean out .png plots.
rm /afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/pdfMaker/plots/*.png
