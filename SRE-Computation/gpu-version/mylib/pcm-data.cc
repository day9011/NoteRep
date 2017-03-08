#include "mylib/pcm-data.h"
namespace kaldi
{

bool PCMGetData::Read(std::string filename, int32 channel, BaseFloat samp_freq)
{
	PCMHolder pcm_holder_;
	Input pcm_input_;
	int fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0)
	{
		KALDI_ERR << "failed to open pcm file";
		return false;
	}
	long fileLen = lseek(fd, 0L, SEEK_END);
	if (!(fileLen > 0))
	{
		KALDI_ERR << "pcm file is empty";
		return false;
	}
	close(fd);
	if (!pcm_input_.Open(filename, NULL))
	{
		KALDI_ERR << "TableReader: failed to open pcm file" << std::endl;
		return false;
	}
	else
	{
		if (pcm_holder_.Read(pcm_input_.Stream(), fileLen))
		{
			KALDI_LOG << "TableReader: successful to open pcm file";
		}
		else
		{
			KALDI_ERR << "TableReader: failed to open pcm file";
			return false;
		}
	}
	const PCMData &data = pcm_holder_.Value();
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
	SubVector<BaseFloat> pcmform(data.Data(), this_chan);
	voice_data_ = pcmform;
	return true;
}


}
