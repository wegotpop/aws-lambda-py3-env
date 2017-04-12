/* NOTE: Since Python may define some pre-processor definitions which affect the
         standard headers on some systems, you must include Python.h before any
         standard headers are included.'
         See: https://docs.python.org/3/c-api/intro.html */
#include <Python.h>
/*  func:   Py_* */

#include <stdio.h>
/*  const:  stderr
    type:   FUNC
    func:   fputs
            fopen
            fclose */
#include <stdlib.h>
/*  const:  EXIT_FAILURE
    func:   malloc
            free */


/* SEE: https://docs.python.org/3/c-api/init.html#c.Py_SetPath */
#if defined(_WIN32) || defined(_WIN64)
    #define POP_PATH_DELIMITER ";"
#else
    #define POP_PATH_DELIMITER ":"
#endif


int
main(int   argc,
     char *argv[])
{
    int       i,
              result = EXIT_FAILURE;
    wchar_t **wargv,
             *home   = Py_DecodeLocale("src", NULL),
             *path   = Py_DecodeLocale("Lib", NULL),
             *python = Py_DecodeLocale("pop_python3", NULL);

    if (!(wargv = malloc((size_t)argc*sizeof(wchar_t*))))
    {
        fputs("Internal error: "
              "Unable to allocate memory for wide-argv\n", stderr);
        goto alloc_failed;
    }

    for (i=0; i<argc; i++)
        wargv[i] = Py_DecodeLocale(argv[i], NULL);

    /* Setup */
    Py_NoSiteFlag = 1;
    Py_NoUserSiteDirectory = 1;
    Py_SetProgramName(python);
    Py_SetPythonHome(home);
    Py_SetPath(path);

    /* Initialise the interpreter */
    Py_InitializeEx(0);

    /* Run main module */
    if ((result = Py_Main(argc, wargv)))
        goto free_resources;

    /* Clean up the interpreter */
    result = Py_FinalizeEx();

    /* Clean up the memory */
    free_resources:
        free(wargv);
        PyMem_RawFree(python);

    alloc_failed:
        return result;
}
