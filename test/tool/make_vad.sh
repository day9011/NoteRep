#!/bin/bash

set -x
set -e

if [ $# -ne 2 ]; then
	echo "Usage: $0 <wavdir:string> <outdir:string>"
	exit 1;
fi


KALDIROOT=~/kaldi

#VOICEFILECONVERT=${KALDI_ROOT}/tools/sph2pipe_v2.5/sph2pipe
FVADWAVCOMMAND=~/libfvad/examples/fvadwav

VOICEFILECONVERTFLAGS="-f wav -p -c 1"
FVADWAVCOMMANDFLAGS="-m 3 -f 30 -o"

VOICEFILEDIR=$1
OUTDIR=$2

[ -d ${OUTDIR} ] || mkdir -p ${OUTDIR} || exit 1;

for x in `find ${VOICEFILEDIR} -type f | grep -E "*.(amr|wav)$"`;
do
	filename=`basename ${x}`
	filepath=`dirname ${x}| sed "s:${VOICEFILEDIR}:${OUTDIR}/:g"`
	[ -d ${filepath} ] || mkdir -p ${filepath} || exit 1;
#no sph2pipe
	if [ ${VOICEFILECONVERT}"x" == "x" ]; then
		sox ${x} -t wav -c 1 - | ${FVADWAVCOMMAND} ${FVADWAVCOMMANDFLAGS} ${filepath}/${filename} -
	else
		${VOICEFILECONVERT} ${VOICEFILECONVERTFLAGS} ${x} | ${FVADWAVCOMMAND} ${FVADWAVCOMMANDFLAGS} ${filepath}/${filename} -
	fi
done


