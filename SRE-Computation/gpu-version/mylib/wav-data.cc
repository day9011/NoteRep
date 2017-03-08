#include "mylib/wav-data.h"
namespace kaldi
{


bool WAVGetData::Read(std::string filename, int32 channel, BaseFloat samp_freq)
{
	WaveHolder wav_holder_;
	Input wav_input_;
	if (!wav_input_.Open(filename, NULL))
	{
		KALDI_ERR << "TableReader: failed to open wav file" << std::endl;
		return false;
	}
	else
	{
		if (wav_holder_.Read(wav_input_.Stream()))
		{
			KALDI_LOG << "TableReader: successful to open wav file";
		}
		else
		{
			KALDI_ERR << "TableReader: failed to open wav file";
			return false;
		}
	}
	const WaveData &data = wav_holder_.Value();
	int32 num_chan = data.Data().NumRows(), this_chan = channel;
	KALDI_ASSERT(num_chan > 0);
	if (channel == -1)
		this_chan = 0;
	else
		if (this_chan >= num_chan)
			return false;
	if (samp_freq != data.SampFreq())
	{
		KALDI_ERR << "Sample frequency mismatch: you specified "
				  << samp_freq << " but data has "
				  << data.SampFreq();
		KALDI_ERR << "Frequency mismatch";
		return false;
	}
	SubVector<BaseFloat> waveform(data.Data(), this_chan);
	voice_data_ = waveform;
	return true;
}

}
