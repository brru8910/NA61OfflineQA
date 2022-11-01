#!/usr/bin/python

#USE: ./haddScript [nPerBatch] [inputDirectory] [outputFilename]
#Script will perform as many hadds as necessary to hadd [nPerBatch]
#files together. It will then merge them as one file and remove the
#intermediates.

import os
import sys
import glob

#Helpoer functions.

#Hadd all files in an input list.
def PerformHadd(fileList,outputFilename):
    command = 'hadd -k -f ' + outputFilename + ' ' + ' '.join(fileList)
    print 'Hadd path: ' + outputFilename + '. Files in batch: ' + str(len(fileList)) + '.'
    os.system(command)

#Remove all files in an input list. Careful with this!
def DeleteFiles(fileList):
    command = 'rm ' + ' '.join(fileList)
    print command
    os.system(command)


#Parse input arguments and return errors if incorrect.
if len(sys.argv) != 5:
    print 'Incorrect usage! Try again. Usage: ./haddScript [nPerBatch] [inputDirectory] [matchPattern] [outputFilename]'
    sys.exit()
    
#Get input arguments. 
nFilesPerBatch = int(sys.argv[1])
directory      = sys.argv[2]
matchPattern   = sys.argv[3]
outputFilename = sys.argv[4]

print 'Directory:'
os.system("pwd")
print 'Performing hadds. Output filename: ' + outputFilename + '. Files per hadd: ' + str(nFilesPerBatch)

#List of files in directory.
#os.chdir(directory)
filenameList = outputFilename + '-Files.txt'
#fileList = glob.glob(directory + '*' + matchPattern)

lsCommand = "ls -t " + directory + "/*" + matchPattern + " > " + filenameList
#lsCommand = "eos ls " + directory + " > " + filenameList
print 'ls command string: ' + lsCommand
os.system(lsCommand)
filenameFile = open(filenameList,"r")
filenameFileContent = filenameFile.read()
fileListUnfiltered = filenameFileContent.split("\n")
substring = matchPattern
#fileList = list(filter(lambda name: substring in name, fileListUnfiltered))
fileList = list(fileListUnfiltered)

#List of temporary hadd files that we will delete later.
temporaryHaddFiles = []

#List of files to hadd and counters.
filesInHaddBatch = []
batchNumber = 0
nFilesInHaddBatch = 0
nTotalFilesAdded = 0

for filename in fileList:
    #filename = directory + "/" + filename
    filesInHaddBatch.append(filename)
    nFilesInHaddBatch += 1
    nTotalFilesAdded += 1
    #If we've hit the max number of files per hadd, or we're at the end of the list, merge the accumulated files and reset.
    if (nFilesInHaddBatch == nFilesPerBatch or nTotalFilesAdded == len(fileList)):
        temporaryFilename = outputFilename + 'Temp' + str(batchNumber)
        PerformHadd(filesInHaddBatch,temporaryFilename)
        batchNumber += 1
        nFilesInHaddBatch = 0
        filesInHaddBatch
        #Clear list (older Python way of doing this...)
        del filesInHaddBatch[:]
        temporaryHaddFiles.append(temporaryFilename)
        
#After we're done, merge the temp files together.
PerformHadd(temporaryHaddFiles,outputFilename)

#Delete the temp files. We're done!
#DeleteFiles(temporaryHaddFiles)
