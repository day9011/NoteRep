#!/bin/bash

data_dir=$1
train_dir=$2
dict_dir=data/local/dict
filter=~/tool/filter_lexicon.py

[ -d ${dict_dir} ] || mkdir -p ${dict_dir} || exit 1;

#cp ${data_dir}/lexicon.txt ${dict_dir} || exit 1;
cp ${data_dir}/lexicon.txt ${dict_dir}/lexicon_cp.txt || exit 1;
python ${filter} ${dict_dir}/lexicon_cp.txt ${dict_dir}/lexicon.txt


echo "sil" > ${dict_dir}/silence_phones.txt
echo "sil" > ${dict_dir}/optional_silence.txt

cat ${dict_dir}/lexicon.txt | awk '{ for(n=2;n<=NF;n++){ phones[$n] = 1; }} END{for (p in phones) print p;}'| \
	sort -u |\
	perl -e '
my %ph_cl;
while (<STDIN>) {
    $phone = $_;
    chomp($phone);
    chomp($_);
    $phone =~ s:([A-Z]+)[0-9]:$1:;
    if (exists $ph_cl{$phone}) { push(@{$ph_cl{$phone}}, $_)  }
	else { $ph_cl{$phone} = [$_]; }
	}
	foreach $key ( keys %ph_cl ) {
	if ($key ne "sil")
	{
		print "@{ $ph_cl{$key} }\n"
	}
}
' | sort -k1 > ${dict_dir}/nonsilence_phones.txt  || exit 1;

cat $dict_dir/silence_phones.txt| awk '{printf("%s ", $1);} END{printf "\n";}' > $dict_dir/extra_questions.txt || exit 1;
cat $dict_dir/nonsilence_phones.txt | perl -e 'while(<>){ foreach $p (split(" ", $_)) {
$p =~ m:^([^\d]+)(\d*)$: || die "Bad phone $_"; $q{$2} .= "$p "; } } foreach $l (values %q) {print "$l\n";}' \
	>> $dict_dir/extra_questions.txt || exit 1;



