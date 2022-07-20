#!/bin/bash

#Clean out stored chunks.

find /afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/EOSDropDirectory/ChunkStorage/ -maxdepth 1 -name "*.pteraw" -delete
