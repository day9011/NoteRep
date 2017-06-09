#include "mylib/sre.h"
#include "mylib/gettime.h"
#include "ivector/plda.h"
#include "mylib/plda-norm.h"
#include "util/common-utils.h"


int main(int argc, char *argv[]) {
	using namespace kaldi;
	typedef kaldi::int32 int32;
	typedef kaldi::int64 int64;
	try {
		const char *usage =
			"Computes PldaNorm score between two iVectors;\n"
			"Usage: wav_score <plda> <train-ivector-rs> <test-ivector-rs> <imporst-mean-rs> <trials> <scores-wxfilename>\n";
		ParseOptions po(usage);
		std::string num_utts_rs;
		PldaConfig plda_opts;
		plda_opts.Register(&po);
		po.Register("num-utts", &num_utts_rs, "Table to read the number of utt");
		po.Read(argc, argv);
		if (po.NumArgs() != 6) {
			po.PrintUsage();
			return -1;
		}
		std::string plda_rx = po.GetArg(1),
					train_ivector_rs = po.GetArg(2),
					test_ivector_rs = po.GetArg(3),
					imposter_mean_ivector_rs = po.GetArg(4),
					trials_rx = po.GetArg(5),
					scores_wx = po.GetArg(6);
		/*
		std::fstream _file;
		_file.open(plda_rx.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find plda file" << std::endl;
			throw false;
		}
		_file.close();
		_file.open(train_ivector_rs.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find train ivector scp file" << std::endl;
			throw false;
		}
		_file.close();
		_file.open(test_ivector_rs.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find test ivector scp file" << std::endl;
			throw false;
		}
		_file.close();
		_file.open(imposter_mean_ivector_rs.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find imposter mean ivector rs file" << std::endl;
			throw false;
		}
		_file.close();
		_file.open(trials_rx.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find trials rs file" << std::endl;
			throw false;
		}
		_file.close();
		_file.open(scores_wx.c_str(), std::ios::in);
		if(!_file) {
			KALDI_WARN << "\ncan't find scores wx file" << std::endl;
			throw false;
		}
		_file.close();
		*/
		Plda plda;
		ReadKaldiObject(plda_rx, &plda);
		PldaNorm plda_norm;
		ReadKaldiObject(plda_rx, &plda_norm);
		int32 dim = plda.Dim();
		Vector<BaseFloat> imposter_mean;
		ReadKaldiObject(imposter_mean_ivector_rs, &imposter_mean);
		SequentialBaseFloatVectorReader train_ivector_reader(train_ivector_rs);
		SequentialBaseFloatVectorReader test_ivector_reader(test_ivector_rs);
		RandomAccessInt32Reader num_utts_reader(num_utts_rs);
		typedef unordered_map<std::string, Vector<BaseFloat>*, StringHasher> HashType;
		HashType train_ivectors, test_ivectors;
		int32 num_train_ivectors = 0;
		for (; !train_ivector_reader.Done(); train_ivector_reader.Next())
		{
			std::string spk = train_ivector_reader.Key();
			if (train_ivectors.count(spk) != 0)
				KALDI_WARN << "Duplicate train ivector found for speaker " << spk;
			const Vector<BaseFloat> &ivector = train_ivector_reader.Value();
			int32 num_examples = 0;
			if (!num_utts_rs.empty())
			{
				if (!num_utts_reader.HasKey(spk))
				{
					KALDI_WARN << "Number of utt not given for speaker " << spk;
					continue;
				}
				num_examples = num_utts_reader.Value(spk);
			}
			else num_examples = 1;
			Vector<BaseFloat> *transformed_ivector = new Vector<BaseFloat>(dim);
			plda.TransformIvector(plda_opts, ivector, num_examples, transformed_ivector);
			train_ivectors[spk] = transformed_ivector;
			++num_train_ivectors;
		}
		KALDI_LOG << "Read " << num_train_ivectors << " training iVectors.";
		if (num_train_ivectors == 0)
			KALDI_WARN << "No training iVectors present.";
		KALDI_LOG << "Reading test iVectors";
		int32 num_test_ivectors = 0;
		for (; !test_ivector_reader.Done(); test_ivector_reader.Next())
		{
			std::string utt = test_ivector_reader.Key();
			if (test_ivectors.count(utt) != 0)
				KALDI_WARN << "Duplicate test iVector found for utt " << utt;
			const Vector<BaseFloat> &ivector = test_ivector_reader.Value();
			Vector<BaseFloat> *transformed_ivector = new Vector<BaseFloat>(dim);
			plda.TransformIvector(plda_opts, ivector, 1, transformed_ivector);
			test_ivectors[utt] = transformed_ivector;
			++num_test_ivectors;
		}
		KALDI_LOG << "Read " << num_test_ivectors << " utt iVectors.";
		if (num_test_ivectors == 0)
			KALDI_WARN << "No test iVectors present.0";
		Input ki(trials_rx);
		bool binary = false;
		Output ko(scores_wx, binary);
		double sum = 0.0, sumsq = 0.0;
		std::string line;
		int64 num_trials_done = 0, num_trials_err = 0;
		// Vector<BaseFloat> *transformed_imposter_mean = new Vector<BaseFloat>(dim);
		// plda.TransformIvector(plda_opts, imposter_mean, 1, transformed_imposter_mean);
		// Vector<double> impost_mean_double(*transformed_imposter_mean);
		// Vector<BaseFloat> impost_acc(dim);
		// impost_acc.SetZero();
		// for (HashType::iterator iter = train_ivectors.begin(); iter != train_ivectors.end(); ++iter)
		//     impost_acc.AddVec(1.0, *(iter->second));
		// impost_acc.Scale(1 / num_train_ivectors);
		// Vector<double> impost_mean_double(impost_acc);
		Vector<double> impost_mean_double(imposter_mean);
		while (std::getline(ki.Stream(), line))
		{
			std::vector<std::string> fields;
			SplitStringToVector(line, " \t\n\r", true, &fields);
			if (fields.size() != 2)
				KALDI_WARN << "Bad line " << (num_trials_done + num_trials_err)
										 << " in input (expected two fields: key1 key2)" << line;
			std::string key1 = fields[0], key2 = fields[1];
			if (train_ivectors.count(key1) == 0)
			{
				KALDI_WARN << "Key " << key1 << " not present in training iVectors.";
				++num_trials_err;
				continue;
			}
			if (test_ivectors.count(key2) == 0)
			{
				KALDI_WARN << "Key " << key2 << " not present in test iVectors.";
				++num_trials_err;
				continue;
			}
			const Vector<BaseFloat> *train_ivector = train_ivectors[key1],
									*test_ivector = test_ivectors[key2];
			Vector<double> train_ivector_dbl(*train_ivector),
						   test_ivector_dbl(*test_ivector);

			int32 num_train_examples;
			if (!num_utts_rs.empty())
				num_train_examples = num_utts_reader.Value(key1);
			else
				num_train_examples = 1;
			double score = plda_norm.ScoreNorm(train_ivector_dbl, num_train_examples, test_ivector_dbl, impost_mean_double);
			sum += score;
			sumsq += score * score;
			++num_trials_done;
			ko.Stream() << key1 << ' ' << key2 << ' ' << score << std::endl;
		}
		for (HashType::iterator iter = train_ivectors.begin(); iter != train_ivectors.end(); ++iter)
			delete iter->second;
		for (HashType::iterator iter = test_ivectors.begin(); iter != test_ivectors.end(); ++iter)
			delete iter->second;

		if (num_trials_done != 0)
		{
			BaseFloat mean = sum / num_trials_done, scatter = sumsq / num_trials_done,
					  variance = scatter - mean * mean, stddev = sqrt(variance);
			KALDI_LOG << "Mean score was " << mean << ", standard deviation was " << stddev;
		}
		KALDI_LOG << "Processed " << num_trials_done << " trials, " << num_trials_err << " had errors.";
		return (num_trials_done != 0 ? 0 : 1);
	} catch(const std::exception &e) {
		std::cerr << e.what();
		std::cout << "error" << std::endl;
		return -1;
	}
}
