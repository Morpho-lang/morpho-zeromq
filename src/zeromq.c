#include <stdio.h>
#include <morpho/morpho.h>
#include <morpho/builtin.h>
#include <morpho/value.h>
#include <morpho/veneer.h>
#include <morpho/functional.h>

#include <czmq.h>
#include "zeromq.h"

/* -------------------------------------------------------
 * ZeroMQ socket object type
 * ------------------------------------------------------- */

typedef struct {
    object obj; 
    zsock_t *socket; 
} objectzeromqsocket; 

objecttype objectzeromqsockettype;
#define ZEROMQ_SOCKET objectzeromqsockettype

/** Gets the object as a zeromq socket */
#define ZEROMQ_GETSOCKET(val)   ((objectzeromqsocket *) MORPHO_GETOBJECT(val))

/** ZeroMQ socket definitions */
void objectzeromqsocket_printfn(object *obj) {
    objectzeromqsocket *d = (objectzeromqsocket *) obj;
    const char *type = zsock_type_str(d->socket);
    printf("<ZeroMQSocket: %s>", type);
}

void objectzeromqsocket_markfn(object *obj, void *v) {
}

void objectzeromqsocket_freefn(object *obj) {
    objectzeromqsocket *sock = (objectzeromqsocket *) obj;
    if (sock->socket) zsock_destroy(&sock->socket);
} 

size_t objectzeromqsocket_sizefn(object *obj) {
    return sizeof(objectzeromqsocket);
}

objecttypedefn objectzeromqsocketdefn = {
    .printfn=objectzeromqsocket_printfn,
    .markfn=objectzeromqsocket_markfn,
    .freefn=objectzeromqsocket_freefn,
    .sizefn=objectzeromqsocket_sizefn
};

/* -------------------------------------------------------
 * ZeroMQ errors
 * ------------------------------------------------------- */

/** Raises an error from ZeroMQ. TODO: Need to make these fine grained so they can be caught */
void zeromq_error(vm *v) {
    morpho_runtimeerror(v, ZEROMQ_ERR, zmq_strerror(errno));
}

/* -------------------------------------------------------
 * Constructor functions 
 * ------------------------------------------------------- */

/** Creates a new ZeroMQ socket */
objectzeromqsocket *object_newzeromqsocket(zsock_t *socket) {
    objectzeromqsocket *new = (objectzeromqsocket *) object_new(sizeof(objectzeromqsocket), ZEROMQ_SOCKET);
    if (new) new->socket = socket; 
    return new; 
}

/** Generic constructor function */
#define ZEROMQ_CONSTRUCTOR(name, consfunc) value ZeroMQ##name (vm *v, int nargs, value *args) { \
    value out = MORPHO_NIL; \
    if (nargs==1 && MORPHO_ISSTRING(MORPHO_GETARG(args, 0))) { \
        zsock_t *sock = consfunc(MORPHO_GETCSTRING(MORPHO_GETARG(args, 0))); \
        if (sock) { \
            objectzeromqsocket *new = object_newzeromqsocket(sock); \
            if (new) { \
                out = MORPHO_OBJECT(new); \
                morpho_bindobjects(v, 1, &out); \
            } else { \
                zsock_destroy(&sock); \
                morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED); \
            } \
        } else zeromq_error(v);\
    } else morpho_runtimeerror(v, ZEROMQ_CONSARGS); \
    return out; \
} 

ZEROMQ_CONSTRUCTOR(Push, zsock_new_push) 
ZEROMQ_CONSTRUCTOR(Pull, zsock_new_pull) 
ZEROMQ_CONSTRUCTOR(Request, zsock_new_req) 
ZEROMQ_CONSTRUCTOR(Reply, zsock_new_rep) 
ZEROMQ_CONSTRUCTOR(Publish, zsock_new_pub) 

value ZeroMQSubscribe(vm *v, int nargs, value *args) { 
    value out = MORPHO_NIL; 
    char *subs = ""; 

    if (nargs>0 && MORPHO_ISSTRING(MORPHO_GETARG(args, 0))) { 
        if (nargs==2 && MORPHO_ISSTRING(MORPHO_GETARG(args, 1))) subs = MORPHO_GETCSTRING(MORPHO_GETARG(args, 1)); 

        zsock_t *sock = zsock_new_sub(MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)), subs); 
        if (sock) { 
            objectzeromqsocket *new = object_newzeromqsocket(sock); 
            if (new) { 
                out = MORPHO_OBJECT(new); 
                morpho_bindobjects(v, 1, &out); 
            } else { 
                zsock_destroy(&sock); 
                morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED); 
            } 
        } else zeromq_error(v);
    } else morpho_runtimeerror(v, ZEROMQ_CONSARGS); 
    return out; 
} 

/* -------------------------------------------------------
 * Veneer class 
 * ------------------------------------------------------- */

/** Send */
value ZeroMQSocket_send(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));

    if (nargs==1 && MORPHO_ISSTRING(MORPHO_GETARG(args, 0))) {
        zstr_send(sock->socket, MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    }

    return MORPHO_NIL;
}

/** Receive */
value ZeroMQSocket_receive(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    value out = MORPHO_NIL; 

    char *string = zstr_recv(sock->socket);
    if (string) {
        out = object_stringfromcstring(string, strlen(string));
        zstr_free(&string);
        if (MORPHO_ISOBJECT(out)) morpho_bindobjects(v, 1, &out);
    }

    return out;
}

/** Determine the endpoint */
value ZeroMQSocket_endpoint(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    value out = MORPHO_NIL; 

    const char *ep = zsock_endpoint(sock->socket);
    if (ep) {
        out = object_stringfromcstring(ep, strlen(ep));
        if (MORPHO_ISOBJECT(out)) morpho_bindobjects(v, 1, &out);
    }

    return out; 
}

/** Subscribe */
value ZeroMQSocket_subscribe(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));

    if (nargs==1 && MORPHO_ISSTRING(MORPHO_GETARG(args, 0))) {
        zsock_set_subscribe(sock->socket, MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    }

    return MORPHO_NIL; 
}

/** Unsubscribe */
value ZeroMQSocket_unsubscribe(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));

    if (nargs==1 && MORPHO_ISSTRING(MORPHO_GETARG(args, 0))) {
        zsock_set_unsubscribe(sock->socket, MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    }

    return MORPHO_NIL; 
}

MORPHO_BEGINCLASS(ZeroMQSocket)
MORPHO_METHOD(ZEROMQ_SEND_METHOD, ZeroMQSocket_send, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_RECEIVE_METHOD, ZeroMQSocket_receive, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_SUBSCRIBE_METHOD, ZeroMQSocket_subscribe, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_UNSUBSCRIBE_METHOD, ZeroMQSocket_unsubscribe, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_ENDPOINT_METHOD, ZeroMQSocket_endpoint, BUILTIN_FLAGSEMPTY)
MORPHO_ENDCLASS

/* -------------------------------------------------------
 * Initialization and finalization
 * ------------------------------------------------------- */

void zeromq_initialize(void) { 
    objectzeromqsockettype=object_addtype(&objectzeromqsocketdefn);
    
    objectstring objclassname = MORPHO_STATICSTRING(OBJECT_CLASSNAME);
    value objclass = builtin_findclass(MORPHO_OBJECT(&objclassname));
    
    value zeromqsocketclass=builtin_addclass(ZEROMQ_SOCKETCLASSNAME, MORPHO_GETCLASSDEFINITION(ZeroMQSocket), objclass);
    object_setveneerclass(ZEROMQ_SOCKET, zeromqsocketclass);

    builtin_addfunction(ZEROMQ_PUSH_CONS, ZeroMQPush, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_PULL_CONS, ZeroMQPull, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_REQUEST_CONS, ZeroMQRequest, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_REPLY_CONS, ZeroMQReply, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_PUBLISH_CONS, ZeroMQPublish, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_SUBSCRIBE_CONS, ZeroMQSubscribe, BUILTIN_FLAGSEMPTY);

    morpho_defineerror(ZEROMQ_CONSARGS, ERROR_HALT, ZEROMQ_CONSARGS_MSG);
    morpho_defineerror(ZEROMQ_ERR, ERROR_HALT, ZEROMQ_ERR_MSG);
}

void zeromq_finalize(void) { 
}
