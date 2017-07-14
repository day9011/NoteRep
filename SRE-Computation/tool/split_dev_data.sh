#!/bin/bash

set -e
set -x

if [ $# -ne 3 ]; then
	echo "Usage: $0 <string:indir> <string:outdir> <string:filetype>"
	exit 1;
fi

indir=$1
outdir=$2
filetype=$3


[ -d ${outdir} ] && rm -rf ${outdir}
mkdir -p ${outdir}/dev

for x in `find ${indir} -type f | grep -E "*.${filetype}$" | sort -u -k1`;
do
	c_spkid=${x#*/}
	c_spkid=${c_spkid%%/*}
	if [ -d ${outdir}/dev/${c_spkid} ];then
		continue
	else
		mkdir -p ${outdir}/dev/${c_spkid} || exit 1;
		mv ${x} ${outdir}/dev/${c_spkid} || exit 1;
	fi
done
