cdef extern from "py-int.h":
    cdef cppclass Testpy:
        Testpy() except +
        Testpy(int) except +
        int n
        int get_n()
        void print_n()
        void set_n(int)

cdef class PyTestpy:
    cdef Testpy* _this

    def __cinit__(self):
        self._this = new Testpy()

    def __cinit__(self, int v):
        self._this = new Testpy(v)

    def __dealloc__(self):
        del self._this

    def get_n(self):
        return self._this.get_n()

    def print_n(self):
        self._this.print_n()

    def set_n(self, int v):
        self._this.set_n(v)
