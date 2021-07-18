#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <sys/time.h>
#include <unistd.h>

timespec gnd;
timespec ref;

const int history_size = 0x40000;
double history[history_size];
int history_idx;

PyObject * meth_init(PyObject * self) {
    clock_gettime(CLOCK_MONOTONIC, &ref);
    Py_RETURN_NONE;
}

PyObject * meth_sync(PyObject * self, PyObject * arg) {
    timespec now = {};
    clock_gettime(CLOCK_MONOTONIC, &now);
    const double timestep = PyFloat_AsDouble(arg);
    const double measured = (double)(now.tv_sec - ref.tv_sec) + (double)(now.tv_nsec - ref.tv_nsec) * 1e-9;
    if (timestep > measured) {
        const long long remaining = (long long)floor((timestep - measured) * 1e9);
        timespec sleep = {remaining / 1000000000, remaining % 1000000000};
        nanosleep(&sleep, NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &ref);
    history[history_idx] = (double)(ref.tv_sec - gnd.tv_sec) + (double)(ref.tv_nsec - gnd.tv_nsec) * 1e-9;
    history_idx = history_idx + 1;
    if (history_idx == history_size) {
        history_idx = 0;
    };
    Py_RETURN_NONE;
}

PyObject * meth_hist(PyObject * self) {
    PyObject * res = PyBytes_FromStringAndSize(NULL, history_size * sizeof(double));
    char * ptr = PyBytes_AsString(res);
    memcpy(ptr, history + history_idx, (history_size - history_idx) * sizeof(double));
    memcpy(ptr + (history_size - history_idx) * sizeof(double), history, history_idx * sizeof(double));
    return res;
}

PyMethodDef module_methods[] = {
    {"init", (PyCFunction)meth_init, METH_NOARGS, NULL},
    {"sync", (PyCFunction)meth_sync, METH_O, NULL},
    {"hist", (PyCFunction)meth_hist, METH_NOARGS, NULL},
    {},
};

PyModuleDef module_def = {PyModuleDef_HEAD_INIT, "time_sync", NULL, -1, module_methods};

extern "C" PyObject * PyInit_time_sync() {
    PyObject * module = PyModule_Create(&module_def);
    return module;
}
