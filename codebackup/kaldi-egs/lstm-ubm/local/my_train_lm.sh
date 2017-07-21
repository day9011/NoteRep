#!/bin/bash

text=$1
lexicon=$2

for f in "$text" "$lexicon"; do
	[ ! -f $x ] && echo "$0: No such file $f" && exit 1;
done

dir=data/local/lm
mkdir -p $dir

export LC_ALL=C # You'll get errors about things being not sorted, if you
# have a different locale.

cleantext=$dir/text.no_oov

cat $text | awk -v lex=$lexicon 'BEGIN{while((getline<lex) >0){ seen[$1]=1; } }
{for(n=2; n<=NF;n++) {  if (seen[$n]) { printf("%s ", $n); } else {printf("<unk> ");} } printf("\n");}' \
	> $cleantext || exit 1;


cat $cleantext | awk '{for(n=2;n<=NF;n++) print $n; }' | sort | uniq -c | \
	sort -nr > $dir/word.counts || exit 1;

echo "finish word counts"

# Get counts from acoustic training transcripts, and add  one-count
# for each word in the lexicon (but not silence, we don't want it
# in the LM-- we'll add it optionally later).
cat $cleantext | awk '{for(n=2;n<=NF;n++) print $n; }' | \
	cat - <(grep -w -v 'sil' $lexicon | awk '{print $1}') | \
	sort | uniq -c | sort -nr > $dir/unigram.counts || exit 1;

echo "finish unigram counts"
# note: we probably won't really make use of <UNK> as there aren't any OOVs
cat $dir/unigram.counts  | awk '{print $2}' | get_word_map.pl "<s>" "</s>" "<unk>" > $dir/word_map \
	|| exit 1;

echo "finish get_word_map.pl"

# note: ignore 1st field of train.txt, it's the utterance-id.
cat $cleantext | awk -v wmap=$dir/word_map 'BEGIN{while((getline<wmap)>0)map[$1]=$2;}
{ for(n=2;n<=NF;n++) { printf map[$n]; if(n<NF){ printf " "; } else { print ""; }}}' | gzip -c >$dir/train.gz \
  	|| exit 1;

echo "gzip train"

train_lm.sh --arpa --lmtype 3gram-mincount $dir || exit 1;

# From here is some commands to do a baseline with SRILM (assuming
# you have it installed).
heldout_sent=10000 # Don't change this if you want result to be comparable with
sdir=$dir/srilm # in case we want to use SRILM to double-check perplexities.
mkdir -p $sdir
cat $cleantext | awk '{for(n=2;n<=NF;n++){ printf $n; if(n<NF) printf " "; else print ""; }}' | \
	head -$heldout_sent > $sdir/heldout
cat $cleantext | awk '{for(n=2;n<=NF;n++){ printf $n; if(n<NF) printf " "; else print ""; }}' | \
	tail -n +$heldout_sent > $sdir/train

cat $dir/word_map | awk '{print $1}' | cat - <(echo "<s>"; echo "</s>" ) > $sdir/wordlist


ngram-count -text $sdir/train -order 3 -limit-vocab -vocab $sdir/wordlist -unk \
	-map-unk "<unk>" -kndiscount -interpolate -lm $sdir/netease.3.lm.gz
ngram -lm $sdir/netease.3.lm.gz -ppl $sdir/heldout
# 0 zeroprobs, logprob= -250954 ppl= 90.5091 ppl1= 132.482

# Note: perplexity SRILM gives to Kaldi-LM model is same as kaldi-lm reports above.
# Difference in WSJ must have been due to different treatment of <UNK>.
ngram -lm $dir/3gram-mincount/lm_unpruned.gz  -ppl $sdir/heldout
# 0 zeroprobs, logprob= -250913 ppl= 90.4439 ppl1= 132.379

# ngram-count -vocab ${lexicon} -text ${text} -order 3 -write ${dir}/netease.3.count -unk || exit 1;
# ngram-count -vocab ${lexicon} -read ${dir}/netease.3.count -order 3 -lm ${dir}/netease.3.lm \
#     -gt1min 3 -gt1max 7 \
#     -gt2min 3 -gt2max 7 \
#     -gt3min 3 -gt3max 7 || exit 1
#
# LM is small enough that we don't need to prune it (only about 0.7M N-grams).
# Perplexity over 128254.000000 words is 90.446690

# note: output is
# data/local/lm/netease.3.lm

exit 0
