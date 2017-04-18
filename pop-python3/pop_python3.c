/* NOTE: Since Python may define some pre-processor definitions which affect the
         standard headers on some systems, you must include Python.h before any
         standard headers are included.'
         See: https://docs.python.org/3/c-api/intro.html */
#include <Python.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/un.h>
#include <sys/socket.h>

#include "constants.h"

static PyObject *PopSocketError;


static PyObject*
pop_pull(PyObject *self,
         PyObject *args)
{
    (void)self;
    (void)args;
    Py_RETURN_NONE;
}


static PyObject*
pop_push(PyObject *self,
         PyObject *args)
{
    (void)self;
    (void)args;

    int    socket_descriptor;
    struct sockaddr_un server_address = {AF_UNIX, SOCKET_SERVER_PATH};

    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        #define CANNOT_CREATE_SOCKET(VALUE)                                    \
            PyErr_SetString(PopSocketError,                                    \
                            "Internal error: "                                 \
                            "cannot create socket (" #VALUE ")");              \
            break;
        /* Raise error */
        switch (errno)
        {
            case EACCES          : CANNOT_CREATE_SOCKET(EACCES)
            case EAFNOSUPPORT    : CANNOT_CREATE_SOCKET(EAFNOSUPPORT)
            case EINVAL          : CANNOT_CREATE_SOCKET(EINVAL)
            case EMFILE          : CANNOT_CREATE_SOCKET(EMFILE)
            case ENOBUFS         : CANNOT_CREATE_SOCKET(ENOBUFS)
            case ENOMEM          : CANNOT_CREATE_SOCKET(ENOMEM)
            case EPROTONOSUPPORT : CANNOT_CREATE_SOCKET(EPROTONOSUPPORT)
            default              : CANNOT_CREATE_SOCKET(unknown)
        }
        #undef CANNOT_CREATE_SOCKET
        goto failed;
    }

    if (connect(socket_descriptor,
                &server_address,
                sizeof(struct sockaddr_un)) == -1)
    {
        #define CANNOT_CONNECT(VALUE)                                          \
            PyErr_SetString(PopSocketError,                                    \
                            "Internal error: "                                 \
                            "cannot connect to server (" #VALUE ")");          \
            break;
        switch (errno)
        {
            case EADDRNOTAVAIL  : CANNOT_CONNECT(EADDRNOTAVAIL)
            case EAFNOSUPPORT   : CANNOT_CONNECT(EAFNOSUPPORT)
            case EALREADY       : CANNOT_CONNECT(EALREADY)
            case EBADF          : CANNOT_CONNECT(EBADF)
            case ECONNREFUSED   : CANNOT_CONNECT(ECONNREFUSED)
            case EINPROGRESS    : CANNOT_CONNECT(EINPROGRESS)
            case EINTR          : CANNOT_CONNECT(EINTR)
            case EISCONN        : CANNOT_CONNECT(EISCONN)
            case ENETUNREACH    : CANNOT_CONNECT(ENETUNREACH)
            case ENOTSOCK       : CANNOT_CONNECT(ENOTSOCK)
            case EPROTOTYPE     : CANNOT_CONNECT(EPROTOTYPE)
            case ETIMEDOUT      : CANNOT_CONNECT(ETIMEDOUT)
            case EIO            : CANNOT_CONNECT(EIO)
            case ELOOP          : CANNOT_CONNECT(ELOOP)
            case ENAMETOOLONG   : CANNOT_CONNECT(ENAMETOOLONG)
            case ENOENT         : CANNOT_CONNECT(ENOENT)
            case ENOTDIR        : CANNOT_CONNECT(ENOTDIR)
            case EACCES         : CANNOT_CONNECT(EACCES)
            case EADDRINUSE     : CANNOT_CONNECT(EADDRINUSE)
            case ECONNRESET     : CANNOT_CONNECT(ECONNRESET)
            case EHOSTUNREACH   : CANNOT_CONNECT(EHOSTUNREACH)
            case EINVAL         : CANNOT_CONNECT(EINVAL)
            case ENETDOWN       : CANNOT_CONNECT(ENETDOWN)
            case ENOBUFS        : CANNOT_CONNECT(ENOBUFS)
            case EOPNOTSUPP     : CANNOT_CONNECT(EOPNOTSUPP)
            default             : CANNOT_CONNECT(unknown)
        }
        #undef CANNOT_CONNECT
        goto clean_up_socket;
    }

    char BUFF[1024];
    // TODO: handle the return value `0`
    if (recv(socket_descriptor, BUFF, 1024, 0) == -1)
    {
        #define CANNOT_RECEIVE(VALUE)                                          \
        PyErr_SetString(PopSocketError,                                        \
                        "Internal error: "                                     \
                        "cannot receive data from server (" #VALUE ")");       \
        break;
        switch (errno)
        {
            case EAGAIN     : CANNOT_RECEIVE(EAGAIN)
            case EBADF      : CANNOT_RECEIVE(EBADF)
            case ECONNRESET : CANNOT_RECEIVE(ECONNRESET)
            case EINTR      : CANNOT_RECEIVE(EINTR)
            case EINVAL     : CANNOT_RECEIVE(EINVAL)
            case ENOTCONN   : CANNOT_RECEIVE(ENOTCONN)
            case ENOTSOCK   : CANNOT_RECEIVE(ENOTSOCK)
            case EOPNOTSUPP : CANNOT_RECEIVE(EOPNOTSUPP)
            case ETIMEDOUT  : CANNOT_RECEIVE(ETIMEDOUT)
            case EIO        : CANNOT_RECEIVE(EIO)
            case ENOBUFS    : CANNOT_RECEIVE(ENOBUFS)
            case ENOMEM     : CANNOT_RECEIVE(ENOMEM)
            default         : CANNOT_RECEIVE(unknown)
        }
        #undef CANNOT_RECEIVE
        goto clean_up_socket;
    }

    puts(BUFF);

    /* If there was a problem */
    clean_up_socket:
        close(socket_descriptor);
    failed:
        Py_RETURN_NONE;
}


static PyMethodDef PopMethods[] =
{
    {"pull",
     pop_pull,
     METH_NOARGS,
     "Receive pickled tuple of the 'event' and 'context' from lambda"},
    {"push",
     pop_push,
     METH_O,
     "Send pickled python object to the parent process (python 2)"},
    {NULL, NULL, 0, NULL}
};


static PyModuleDef PopModule =
{
    PyModuleDef_HEAD_INIT,
    "_aws_lambda",
    NULL,
    -1,
    PopMethods,
    NULL,
    NULL,
    NULL,
    NULL
};


static PyObject*
PyInit_Pop(void)
{
    PyObject *module;

    if (!(module = PyModule_Create(&PopModule)))
        return NULL;

    /* Create module-level exception */
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-qual"
    PopSocketError = PyErr_NewException(
        (char *)"_aws_lambda.PopSocketError", NULL, NULL);
    #pragma GCC diagnostic pop
    Py_INCREF(PopSocketError);
    PyModule_AddObject(module, "PopSocketError", PopSocketError);

    return module;
}


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

    /* Extend with the custom module */
    PyImport_AppendInittab("_aws_lambda", &PyInit_Pop);

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
