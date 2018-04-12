#include "mylib/sre.h"
#include "stdlib.h"
#include "time.h"

int main(int argc, char *argv[]){
	srand((unsigned)time(NULL));
	using kaldi::int32;
	char *usage = "./productfloat num";
	ParseOptions po(usage);
	po.Read(argc, argv);
	if (po.NumArgs() != 1) {
		po.PrintUsage();
		exit(1);
	}
	std::string num_str = po.GetArg(1);
	int32 num = std::stoi(num_str);
	std::cout << "[";
	for (int32 i = 0;i < num;i++){
		BaseFloat f = (BaseFloat)(rand()%10000)/10000.0f;
		std::cout << " " << f;
	}
	std::cout << " ]";
}
