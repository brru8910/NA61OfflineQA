#!/bin/bash

if [[ $# -gt 1 ]]
then
    echo "Incorrect usage! Usage: ./CreateQAPDF (run number, properly zero-padded)"
    exit 1
fi

runNumber=$1

EOSDropDirectory='/afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/EOSDropDirectory'
qaDir=$EOSDropDirectory'/GEM-BPD'
pdfMakerDirectory='/afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/pdfMaker'

cd $qaDir

#List of QA rootfile template names.
qaFiles=("QA")

for qaName in "${qaFiles[@]}"
do
    matchString=''
    if [[ ${#runNumber} -ne 0 ]]
    then
	matchString='GEMBPD-run'$runNumber'x*'$qaName'.root'
    else
	matchString='*'$qaName'.root'
    fi
    echo "qaName: "$qaName 
    echo 'Match string: '$matchString
    #Get QA histogram files.
    echo 'command: ls -rt '$qaDir'/'$matchString' > haddList'$qaName'.txt'
    ls -rt ./$matchString > haddList$qaName.txt
    #hadd files.
    hadd -k -f $qaName.root `cat haddList$qaName.txt`
    #Export plots to PNG files.
    cd $pdfMakerDirectory
    root -b -q 'ProcessGEMBPDPlots.C("'$qaDir'/'$qaName'.root")'
    #Remove hadd'ed files and lists of files.
    cd $qaDir
    rm $qaName.root
    rm haddList$qaName.txt
done

#Generate PDF.
runString=''
if [[ ${#runNumber} -ne 0 ]]
then
    runString='run-'$runNumber'-'
fi

cd $pdfMakerDirectory
pdflatex GEMBPDQASlides.tex
qaPDFName="GEMBPDQA-"$runString`date +"%Y-%m-%d"`"-"`date +"%H.%M.%S"`".pdf"
mv GEMBPDQASlides.pdf $EOSDropDirectory'/QAPDFs/GEM-BPD/'$qaPDFName

#Clean up.
rm *.aux *.log *.nav *.out *.snm *.toc

exit 0
