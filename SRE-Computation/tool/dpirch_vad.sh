#!/bin/bash

set -x

KALDIROOT=~/Documents/kaldi

VOICEFILECONVERT=${KALDI_ROOT}/tools/sph2pipe_v2.5/sph2pipe
FVADWAVCOMMAND=~/Documents/libfvad/examples/fvadwav

VOICEFILECONVERTFLAGS="-f wav -p -c 1"
FVADWAVCOMMANDFLAGS="-m 3 -f 30 -o"

VOICEFILEDIR=$1

for x in `find ${VOICEFILEDIR} -type f | grep -E "*.(amr|wav)"`;
do
	filename=`basename ${x}`
	filepath=${x%/*}
	filename=${filename/_/_vaded_}
#	`${VOICEFILECONVERT} ${VOICEFILECONVERTFLAGS} ${x} | ${FVADWAVCOMMAND} ${FVADWAVCOMMANDFLAGS} ${filepath}/${filename} -`
#no sph2pipe
	${FVADWAVCOMMAND} ${FVADWAVCOMMANDFLAGS} ${filepath}/${filename} ${x}
done


