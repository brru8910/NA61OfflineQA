#!/bin/bash

#Number of most-recent files to hadd. 
numberOfFiles=2000

if [[ $# -gt 1 ]]
then
    echo "Incorrect usage! Usage: ./CreateQAPDF [optional run number, properly zero-padded]"
    exit 1
fi

runNumber=$1

EOSDropDirectory='/afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/EOSDropDirectory'
pdfMakerDirectory='/afs/cern.ch/user/n/na61qa/2022-p+T2K-OfflineQA/pdfMaker'

cd $EOSDropDirectory

#List of QA rootfile template names.
qaFiles=("TPCClusterQA" "TPCVertexQA" "TDAQQA" "TOFFQA" "GRCClusterQA")

for qaName in "${qaFiles[@]}"
do
    matchString=''
    if [[ ${#runNumber} -ne 0 ]]
    then
	matchString='run-'$runNumber'x*.'$qaName'.root'
    else
	matchString='*'$qaName'.root'
    fi
    echo "qaName: "$qaName 
    echo 'Match string: '$matchString
    #Get QA histogram files.
    echo 'command: ls -rt '$EOSDropDirectory'/'$matchString' | tail -n '$numberOfFiles' | sort > haddList'$qaName'.txt'
    ls -rt $EOSDropDirectory'/'$matchString | tail -n $numberOfFiles | sort > haddList$qaName.txt
    #hadd files.
    hadd -k -f $qaName.root `cat haddList$qaName.txt`
    #Export plots to PNG files.
    cd $pdfMakerDirectory
    root -b -q 'ProcessPlots.C("'$EOSDropDirectory'/'$qaName'.root")'
    #Remove hadd'ed files and lists of files.
    cd $EOSDropDirectory
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
pdflatex OfflineQASlides.tex
qaPDFName="OfflineQA-"$runString`date +"%Y-%m-%d"`"-"`date +"%H.%M.%S"`".pdf"
mv OfflineQASlides.pdf $EOSDropDirectory'/QAPDFs/General/'$qaPDFName

#Clean up.
rm *.aux *.log *.nav *.out *.snm *.toc

exit 0
