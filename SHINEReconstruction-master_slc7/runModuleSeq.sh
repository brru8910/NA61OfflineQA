#!/bin/bash

#################### User definitions section ##########################

INTERNALOUTPUTS="TPCClusterQA.root TPCVertexQA.root TDAQQA.root TOFFQA.root"

#################### Enf of user definitions section ###################

function usage {
  echo """
  usage: $0 <option>
    available options:
      -i <INPUTFILE> (required)
      -o <OUTPUTFILEPREFIX> (required)
      -b <BOOTSTRAPFILE> (required, .in file must exist)
      -v <VERTEXFIT> (required, \"pp\" or \"pA\")
      -k <GLOBALKEY> (required)
      -m <MAGFIELD> (optional, \"on\" or \"off\", default=\"on\")\n

  example:
    $0 -i run-018449x001.raw -o run-018449x001 -b bootstrap.xml -k 14_001 -v pp -m on
  will produce run-018449x001.<something>.root\n

  * bootstrap files with "_static" prefix may use fixed calibration constants
    as fallback (only recommended for test production)
  * bootstrap files will be created from the ".in" version, please make sure to provide one. All
    changes in bootstrap.xml (without .in) will be overwritten
  """
}

# Reset in case getopts has been used previously in the shell.
OPTIND=1

# Initialize our own variables:
INPUTFILE=""
OUTPUTFILEPREFIX=""
BOOTSTRAPFILE=""
GLOBALKEY=""
VERTEXFIT=""
MAGFIELDOFF="-on"

while getopts ":hi:o:b:i:k:m:v:" opt; do
  case $opt in
    h)
      usage
      exit 1
      ;;
    o)
      OUTPUTFILEPREFIX=$OPTARG
      ;;
    b)
      BOOTSTRAPFILE=$OPTARG
      ;;
    i)
      INPUTFILE=$OPTARG
      ;;
    k)
      GLOBALKEY=$OPTARG
      ;;
    m)
      MAGFIELDOFF=-$OPTARG
      ;;
    v)
      VERTEXFIT=-$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done

### Get settings from command line argument.

if [[ "$INPUTFILE" == "" ]]  ; then echo "No input file name given!" 1>&2  ; usage; exit 1 ; fi
if [[ "$GLOBALKEY" == "" ]]  ; then echo "No global key given!" 1>&2  ; usage; exit 1 ; fi
if ! [[ -f $BOOTSTRAPFILE.in ]] ; then echo "File $BOOTSTRAPFILE.in not existing as a local file (needed to create $BOOTSTRAPFILE)!" 1>&2 ; usage; exit 1 ; fi
if ! [[ -f $INPUTFILE ]] ; then echo "Input file $INPUTFILE not existing as a local file!" 1>&2 ; usage; exit 1 ; fi
if [[ "$OUTPUTFILEPREFIX" == "" ]] ; then echo "No output prefix name given!" 1>&2 ; usage; exit 1 ; fi
if [[ "$VERTEXFIT" == "" ]] ; then echo "No vertex fit option given!" 1>&2 ; usage; exit 1 ; fi

### recreate bootstrap file
rm $BOOTSTRAPFILE
make -s

### set global key
sed -i -e 's/@GLOBALKEY@/'"$GLOBALKEY"'/g' $BOOTSTRAPFILE
sed -i -e 's/@VERTEXFIT@/'"$VERTEXFIT"'/g' $BOOTSTRAPFILE
sed -i -e 's/@MAGFIELDOFF@/'"$MAGFIELDOFF"'/g' $BOOTSTRAPFILE

echo "------------------------------"
echo " global key is $GLOBALKEY"
echo " vertex option $VERTEXFIT"
echo " magnetic field option $MAGFIELDOFF"
echo "------------------------------"

if ! [[ -f $BOOTSTRAPFILE ]] ; then echo "Bootstrap file $BOOTSTRAPFILE not existing as a local file!" 1>&2 ; usage; exit 1 ; fi

### Export input file so that it can be read from EventFileReader.xml
export INPUTFILE

### Run the Shine module sequence.
ipcs -l
ShineOffline -b $BOOTSTRAPFILE
EXITCODE=$?

### just to make sure, kill the ds server
ds kill > /dev/null 2>&1

if (( $EXITCODE != 0 )) ; then echo "Shine exited with exit code $EXITCODE !" 1>&2 ; exit $EXITCODE ; fi

### Save output under appropriate name.
for INTERNALOUTPUT in $INTERNALOUTPUTS ; do
  if ! [[ -f $INTERNALOUTPUT ]] ; then
    echo "Calibration output file $INTERNALOUTPUT hasn't been generated for $INPUTFILE !" 1>&2
  else
    mv $INTERNALOUTPUT ${OUTPUTFILEPREFIX}.${INTERNALOUTPUT}
  fi
done
