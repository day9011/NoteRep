#!/bin/bash

set -x
set -e

if [ $# -ne 1 ]; then
	echo "Usage: $0 <amrDir:string>"
	exit 1;
fi

amrDir=$1

for x in `find ${amrDir} -type f | grep -E "*.amr"`;
do
	filename=`basename ${x}`
	path=`dirname ${x}`
	name=${filename/.*}
	$(sox ${x} -t wav - | sox - -b 16 -r 8000 ${path}/${name}.wav)
done
