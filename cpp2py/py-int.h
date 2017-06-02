#include <iostream>

class Testpy
{
	private:
		int n;
	public:
		Testpy() { n = 0; }
		Testpy(int value);
		int get_n();
		void print_n();
		void set_n(int v);

		~Testpy() {}
};

inline int call_get_n(Testpy* ins)
{
	return ins->get_n();
}

inline void call_print_n(Testpy* ins)
{
	ins->print_n();
}

inline void call_set_n(Testpy* ins, int v)
{
	ins->set_n(v);
}

