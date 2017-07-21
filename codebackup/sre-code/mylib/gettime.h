#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef GET_TIME
#define GET_TIME

inline long long getSystemTime()
{
	struct timeb t;
	ftime(&t);
	return 1000 * t.time + t.millitm;
}

class my_time
{
	private:
		long long int start_t;
		long long int end_t;
	public:
		my_time() : start_t(0), end_t(0) {}
		~my_time() {}

		void start()
		{
			start_t = getSystemTime();
		}

		void end()
		{
			end_t = getSystemTime();
		}

		long long int used_time()
		{
			return end_t - start_t;
		}
};

#endif
