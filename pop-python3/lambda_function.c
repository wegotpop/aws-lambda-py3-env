/* NOTE: Since Python may define some pre-processor definitions which affect the
         standard headers on some systems, you must include Python.h before any
         standard headers are included.'
         See: https://docs.python.org/3/c-api/intro.html */
#include <Python.h>
/*  func:   Py_* */

#include <stdlib.h>
/*  func:   system */


static PyObject *PopPython3Error;


static PyObject*
lambda_handler(PyObject *self,
               PyObject *args)
{
    //
    // To handle *args, **kwargs => PyArg_ParseTupleAndKeywords()
    //
    (void)self;
    (void)args;
    /* If there was a problem */
    if (system("./pop_python3 src/main.py"))
    {
        PyErr_SetString(PopPython3Error,
                        "System call returned non-zero exit code");
        return NULL;
    }
    /* If everything goes well, return None */
    Py_RETURN_NONE;
}


static PyMethodDef AWSLambdaFunctions[] =
{
    {"lambda_handler",
     lambda_handler,
     METH_VARARGS|METH_KEYWORDS,
     "AWS entry point."},
    {NULL, NULL, 0, NULL}
};


PyMODINIT_FUNC
initlambda_function(void)
{
    /* Create module */
    PyObject *module;
    if (!(module = Py_InitModule("lambda_function", AWSLambdaFunctions)))
        return;

    /* Create module-level exception */
    PopPython3Error = PyErr_NewException(
        "lambda_function.PopPython3Error", NULL, NULL);
    Py_INCREF(PopPython3Error);
    PyModule_AddObject(module, "PopPython3Error", PopPython3Error);
}
