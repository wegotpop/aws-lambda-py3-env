/* NOTE: Since Python may define some pre-processor definitions which affect the
         standard headers on some systems, you must include Python.h before any
         standard headers are included.'
         See: https://docs.python.org/3/c-api/intro.html */
#include <Python.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "constants.h"


#define POP_ENTRY_POINT "src/main.py"

static PyObject *PopPython3Error;
// TODO: create special errors, like PopInternalError, instead of adding the
//       "Internal error:" prefix to each message


static void*
communication_thread(void *pickled_arguments)
{
    char       *buffer = NULL;
    Py_ssize_t  buffer_size;
    int         socket_descriptor,
                accept_descriptor;
    struct      sockaddr_un server_address = {AF_UNIX, SOCKET_SERVER_PATH};

    /* Make sure the socket is not bound */
    unlink(SOCKET_SERVER_PATH);

    /* Creat nameless socket */
    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        #define CANNOT_CREATE_SOCKET(VALUE)                                    \
            PyErr_SetString(PopPython3Error,                                   \
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

    /* Bind socket */
    if (bind(socket_descriptor,
             &server_address,
             sizeof(struct sockaddr_un)) == -1)
    {
        #define CANNOT_BIND_SOCKET(VALUE)                                      \
            PyErr_SetString(PopPython3Error,                                   \
                            "Internal error: "                                 \
                            "cannot bind socket (" #VALUE ")");                \
            break;
        /* Raise error */
        switch (errno)
        {
            case EACCES        : CANNOT_BIND_SOCKET(EACCES)
            case EADDRINUSE    : CANNOT_BIND_SOCKET(EADDRINUSE)
            case EBADF         : CANNOT_BIND_SOCKET(EBADF)
            case EINVAL        : CANNOT_BIND_SOCKET(EINVAL)
            case ENOTSOCK      : CANNOT_BIND_SOCKET(ENOTSOCK)
            case EADDRNOTAVAIL : CANNOT_BIND_SOCKET(EADDRNOTAVAIL)
            case EFAULT        : CANNOT_BIND_SOCKET(EFAULT)
            case ELOOP         : CANNOT_BIND_SOCKET(ELOOP)
            case ENAMETOOLONG  : CANNOT_BIND_SOCKET(ENAMETOOLONG)
            case ENOENT        : CANNOT_BIND_SOCKET(ENOENT)
            case ENOMEM        : CANNOT_BIND_SOCKET(ENOMEM)
            case ENOTDIR       : CANNOT_BIND_SOCKET(ENOTDIR)
            case EROFS         : CANNOT_BIND_SOCKET(EROFS)
            default            : CANNOT_BIND_SOCKET(unknown)
        }
        #undef CANNOT_BIND_SOCKET
        goto clean_up_socket;
    }

    #define MAX_CONNECTION_COUNT 10
    if (listen(socket_descriptor, MAX_CONNECTION_COUNT) == -1)
    {
        #define CANNOT_MARK_AS_PASSIVE(VALUE)                                  \
            PyErr_SetString(PopPython3Error,                                   \
                            "Internal error: "                                 \
                            "cannot mark socket as passive (" #VALUE ")");     \
            break;
        /* Raise error */
        switch (errno)
        {
            case EADDRINUSE : CANNOT_MARK_AS_PASSIVE(EADDRINUSE)
            case EBADF      : CANNOT_MARK_AS_PASSIVE(EBADF)
            case ENOTSOCK   : CANNOT_MARK_AS_PASSIVE(ENOTSOCK)
            case EOPNOTSUPP : CANNOT_MARK_AS_PASSIVE(EOPNOTSUPP)
            default         : CANNOT_MARK_AS_PASSIVE(unknown)
        }
        #undef CANNOT_MARK_AS_PASSIVE
        goto clean_up_socket;
    }
    #undef MAX_CONNECTION_COUNT

    if ((accept_descriptor = accept(socket_descriptor, NULL, NULL)) == -1)
    {
        #define CANNOT_ACCEPT_CONNECTION(VALUE)                                \
            PyErr_SetString(PopPython3Error,                                   \
                            "Internal error: "                                 \
                            "cannot accept connection (" #VALUE ")");          \
            break;
        /* Raise error */
        switch (errno)
        {
            case EAGAIN       : CANNOT_ACCEPT_CONNECTION(EAGAIN)
            case EBADF        : CANNOT_ACCEPT_CONNECTION(EBADF)
            case ECONNABORTED : CANNOT_ACCEPT_CONNECTION(ECONNABORTED)
            case EFAULT       : CANNOT_ACCEPT_CONNECTION(EFAULT)
            case EINTR        : CANNOT_ACCEPT_CONNECTION(EINTR)
            case EINVAL       : CANNOT_ACCEPT_CONNECTION(EINVAL)
            case EMFILE       : CANNOT_ACCEPT_CONNECTION(EMFILE)
            case ENOMEM       : CANNOT_ACCEPT_CONNECTION(ENOMEM)
            case ENOTSOCK     : CANNOT_ACCEPT_CONNECTION(ENOTSOCK)
            case EOPNOTSUPP   : CANNOT_ACCEPT_CONNECTION(EOPNOTSUPP)
            case EPROTO       : CANNOT_ACCEPT_CONNECTION(EPROTO)
            case EPERM        : CANNOT_ACCEPT_CONNECTION(EPERM)
            default           : CANNOT_ACCEPT_CONNECTION(unknown)
        }
        #undef CANNOT_ACCEPT_CONNECTION
        goto clean_up_socket;
    }

    Py_INCREF((PyObject *)pickled_arguments);

    if ((buffer_size = PyString_Size((PyObject *)pickled_arguments)) < 0)
    {
        PyErr_SetString(PopPython3Error,
                        "Internal error: buffer size is negative");
        goto clean_up_socket;
    }

    if (!(buffer = PyString_AsString((PyObject *)pickled_arguments)))
    {
        PyErr_SetString(PopPython3Error,
                        "Internal error: buffer is NULL");
        goto clean_up_socket;
    }


    ///
    printf("[send] buffer_size: %zu\n", buffer_size);
    printf("[send] buffer: %s\n", buffer);
    ///


    if (send(accept_descriptor, buffer, (size_t)buffer_size, 0) == -1)
    {
        #define CANNOT_SEND_DATA(VALUE)                                        \
            PyErr_SetString(PopPython3Error,                                   \
                            "Internal error: "                                 \
                            "cannot send data (" #VALUE ")");                  \
            break;
        /* Raise error */
        switch (errno)
        {
            case EACCES       : CANNOT_SEND_DATA(EACCES)
            case EAGAIN       : CANNOT_SEND_DATA(EAGAIN)
            case EBADF        : CANNOT_SEND_DATA(EBADF)
            case ECONNRESET   : CANNOT_SEND_DATA(ECONNRESET)
            case EDESTADDRREQ : CANNOT_SEND_DATA(EDESTADDRREQ)
            case EFAULT       : CANNOT_SEND_DATA(EFAULT)
            case EINTR        : CANNOT_SEND_DATA(EINTR)
            case EINVAL       : CANNOT_SEND_DATA(EINVAL)
            case EISCONN      : CANNOT_SEND_DATA(EISCONN)
            case EMSGSIZE     : CANNOT_SEND_DATA(EMSGSIZE)
            case ENOBUFS      : CANNOT_SEND_DATA(ENOBUFS)
            case ENOMEM       : CANNOT_SEND_DATA(ENOMEM)
            case ENOTCONN     : CANNOT_SEND_DATA(ENOTCONN)
            case ENOTSOCK     : CANNOT_SEND_DATA(ENOTSOCK)
            case EOPNOTSUPP   : CANNOT_SEND_DATA(EOPNOTSUPP)
            case EPIPE        : CANNOT_SEND_DATA(EPIPE)
            default           : CANNOT_SEND_DATA(unknown)
        }
        #undef CANNOT_SEND_DATA
        goto clean_up_accept;
    }

    /* Clean up */
    Py_DECREF((PyObject *)pickled_arguments);
    close(accept_descriptor);
    close(socket_descriptor);
    unlink(SOCKET_SERVER_PATH);

    /* If everything went fine */
    Py_RETURN_NONE;

    /* If there was a problem */
    clean_up_accept:
        close(accept_descriptor);
    clean_up_socket:
        close(socket_descriptor);
        unlink(SOCKET_SERVER_PATH);
    failed:
        return NULL;
}


static PyObject*
lambda_handler(PyObject *self,
               PyObject *arguments)
{
    (void)self;

    int        result;
    pthread_t  thread;
    PyObject  *send_pickled,
              *recv_pickled,
              *module,
              *dumps_method,
              *loads_method,
              *protocol,
              *received;

    /* Import pickle module */
    if (!(module = PyImport_ImportModuleNoBlock("cPickle")))
        goto failed;

    /* Create method argument */
    if (!(dumps_method = PyString_FromString("dumps")))
        goto failed;

    /* Create protocol argument */
    if (!(protocol = PyInt_FromLong(HIGHEST_PYTHON2_PROTOCOL)))
        goto failed;

    /* Pickle the lambda_handler arguments */
    Py_INCREF(module);
    Py_INCREF(dumps_method);
    Py_INCREF(arguments);
    Py_INCREF(protocol);
    if (!(send_pickled = PyObject_CallMethodObjArgs(module,
                                                    dumps_method,
                                                    arguments,
                                                    protocol,
                                                    NULL)))
        goto failed;

    /* Create the server side of the communication in a separate thread */
    Py_INCREF(send_pickled);
    if ((result = pthread_create(&thread,
                                 NULL,
                                 communication_thread,
                                 (void *)send_pickled)))
    {
        #define CANNOT_CREATE_THREAD(VALUE)                                    \
            PyErr_SetString(PopPython3Error,                                   \
                            "Internal error: "                                 \
                            "cannot create thread (" #VALUE ")");              \
            break;
        /* Raise error */
        switch (result)
        {
           case EAGAIN : CANNOT_CREATE_THREAD(EAGAIN)
           case EINVAL : CANNOT_CREATE_THREAD(EINVAL)
           case EPERM  : CANNOT_CREATE_THREAD(EPERM)
           default     : CANNOT_CREATE_THREAD(unknown)
        }
        #undef CANNOT_CREATE_THREAD
        goto failed;
    }

    /* Run the custom python interpreter */
    if (system("./pop_python3 " POP_ENTRY_POINT))
    {
        PyErr_SetString(PopPython3Error,
                        "Internal error: something went wrong in '"
                        POP_ENTRY_POINT "'");
        goto failed;
    }

    /* Wait for communication thread to return */
    if ((result = pthread_join(thread, (void *)&recv_pickled)))
    {
        #define CANNOT_JOIN_THREAD(VALUE)                                      \
            PyErr_SetString(PopPython3Error,                                   \
                            "Internal error: "                                 \
                            "cannot join thread (" #VALUE ")");                \
            break;
        switch (result)
        {
            case EDEADLK : CANNOT_JOIN_THREAD(EDEADLK)
            case EINVAL  : CANNOT_JOIN_THREAD(EINVAL)
            case ESRCH   : CANNOT_JOIN_THREAD(ESRCH)
            default      : CANNOT_JOIN_THREAD(unknown)
        }
        #undef CANNOT_JOIN_THREAD
        goto failed;
    }

    // /* If something went wrong inside the thread */
    // if (!recv_pickled)
    //     goto failed;

    // /* Create method argument */
    // if (!(loads_method = PyString_FromString("loads")))
    //     goto failed;

    // /* Pickle the lambda_handler arguments */
    // Py_INCREF(loads_method);
    // Py_INCREF(recv_pickled);
    // if (!(received = PyObject_CallMethodObjArgs(module,
    //                                             loads_method,
    //                                             recv_pickled,
    //                                             NULL)))
    //     goto failed;

    (void)loads_method;

    Py_DECREF(module);
    Py_DECREF(dumps_method);
    Py_DECREF(arguments);
    Py_DECREF(protocol);
    Py_DECREF(send_pickled);
    // Py_DECREF(loads_method);
    // Py_DECREF(recv_pickled);

    received = Py_None;

    /* If everything went fine, return the result value */
    Py_INCREF(received);
    return received;

    /* If something went wrong */
    failed:
        return NULL;
}


static PyMethodDef AWSLambdaFunctions[] =
{
    {"lambda_handler",
     lambda_handler,
     METH_VARARGS,
     "AWS entry point."},
    {NULL, NULL, 0, NULL}
};


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
PyMODINIT_FUNC
initlambda_function(void)
{
    /* Create module */
    PyObject *module;
    if (!(module = Py_InitModule("lambda_function", AWSLambdaFunctions)))
        return;

    /* Create module-level exception */
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wcast-qual"
    PopPython3Error = PyErr_NewException(
        (char *)"lambda_function.PopPython3Error", NULL, NULL);
    #pragma GCC diagnostic pop
    Py_INCREF(PopPython3Error);
    PyModule_AddObject(module, "PopPython3Error", PopPython3Error);
}
#pragma GCC diagnostic pop
