#!/bin/bash

set -x

amrDir=~/Data/audio
wavDir=~/Data/amr_test/amrWav

for x in `find ${amrDir} -type f | grep -E "*.amr"`;
do
	filename=`basename ${x}`
	name=${filename/.*}
	$(sox ${x} -t wav ${wavDir}/${name}.wav)
done
