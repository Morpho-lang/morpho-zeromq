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

/** Tests whether an object is an socket */
#define ZEROMQ_ISSOCKET(val) object_istype(val, ZEROMQ_SOCKET)

/** ZeroMQ socket definitions */
void objectzeromqsocket_printfn(object *obj) {
    objectzeromqsocket *d = (objectzeromqsocket *) obj;
    const char *type = zsock_type_str(d->socket);
    printf("<%s: %s>", ZEROMQ_SOCKETCLASSNAME, type);
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
 * ZeroMQ poller object type
 * ------------------------------------------------------- */

typedef struct {
    object obj; 
    zpoller_t *poller; 
    dictionary readers; 
} objectzeromqpoller; 

objecttype objectzeromqpollertype;
#define ZEROMQ_POLLER objectzeromqpollertype

/** Tests whether an object is an poller */
#define ZEROMQ_ISPOLLER(val) object_istype(val, ZEROMQ_POLLER)

/** Gets the object as a zeromq poller */
#define ZEROMQ_GETPOLLER(val)   ((objectzeromqpoller *) MORPHO_GETOBJECT(val))

/** ZeroMQ poller definitions */
void objectzeromqpoller_printfn(object *obj) {
    printf("<ZeroMQPoller>");
}

void objectzeromqpoller_markfn(object *obj, void *v) {
    objectzeromqpoller *poller = (objectzeromqpoller *) obj;
    for (int i=0; i<poller->readers.capacity; i++) {
        dictionaryentry *e = poller->readers.contents;
        if (!MORPHO_ISNIL(e->key)) morpho_markvalue(v, e->val);
    }
}

void objectzeromqpoller_freefn(object *obj) {
    objectzeromqpoller *poller = (objectzeromqpoller *) obj;
    if (poller->poller) zpoller_destroy(&poller->poller);
    dictionary_clear(&poller->readers);
} 

size_t objectzeromqpoller_sizefn(object *obj) {
    return sizeof(objectzeromqpoller);
}

objecttypedefn objectzeromqpollerdefn = {
    .printfn=objectzeromqpoller_printfn,
    .markfn=objectzeromqpoller_markfn,
    .freefn=objectzeromqpoller_freefn,
    .sizefn=objectzeromqpoller_sizefn
};

/* -------------------------------------------------------
 * ZeroMQ proxy object type
 * ------------------------------------------------------- */

typedef struct {
    object obj; 
    zactor_t *proxy; 
} objectzeromqproxy; 

objecttype objectzeromqproxytype;
#define ZEROMQ_PROXY objectzeromqproxytype

/** Tests whether an object is an poller */
#define ZEROMQ_ISPROXY(val) object_istype(val, ZEROMQ_PROXY)

/** Gets the object as a zeromq proxy */
#define ZEROMQ_GETPROXY(val)   ((objectzeromqproxy *) MORPHO_GETOBJECT(val))

/** ZeroMQ proxy definitions */
void objectzeromqproxy_printfn(object *obj) {
    printf("<ZeroMQProxy>");
}

void objectzeromqproxy_markfn(object *obj, void *v) {
//    objectzeromqproxy *proxy = (objectzeromqproxy *) obj;
}

void objectzeromqproxy_freefn(object *obj) {
    objectzeromqproxy *proxy = (objectzeromqproxy *) obj;
    if (proxy->proxy) zactor_destroy(&proxy->proxy);
} 

size_t objectzeromqproxy_sizefn(object *obj) {
    return sizeof(objectzeromqproxy);
}

objecttypedefn objectzeromqproxydefn = {
    .printfn=objectzeromqproxy_printfn,
    .markfn=objectzeromqproxy_markfn,
    .freefn=objectzeromqproxy_freefn,
    .sizefn=objectzeromqproxy_sizefn
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
    char *endpoint = NULL; \
    if (nargs==1 && MORPHO_ISSTRING(MORPHO_GETARG(args, 0))) { \
        endpoint = MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)); \
    } else if (nargs!=0) morpho_runtimeerror(v, ZEROMQ_CONSARGS); \
    zsock_t *sock = consfunc(endpoint); \
    if (sock) { \
        objectzeromqsocket *new = object_newzeromqsocket(sock); \
        if (new) { \
            out = MORPHO_OBJECT(new); \
            morpho_bindobjects(v, 1, &out); \
        } else { \
            zsock_destroy(&sock); \
            morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED); \
        } \
    } else zeromq_error(v); \
    return out; \
} 

ZEROMQ_CONSTRUCTOR(Publisher, zsock_new_pub) 

value ZeroMQSubscriber(vm *v, int nargs, value *args) { 
    value out = MORPHO_NIL;
    char *subs = "";
    char *endpoint = NULL;

    if (nargs>0 && MORPHO_ISSTRING(MORPHO_GETARG(args, 0))) { 
        endpoint = MORPHO_GETCSTRING(MORPHO_GETARG(args, 0));
    } else morpho_runtimeerror(v, ZEROMQ_CONSARGS);
    
    if (nargs==2 && MORPHO_ISSTRING(MORPHO_GETARG(args, 1))) subs = MORPHO_GETCSTRING(MORPHO_GETARG(args, 1)); 

    zsock_t *sock = zsock_new_sub(endpoint, subs); 
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
    return out; 
} 

ZEROMQ_CONSTRUCTOR(Request, zsock_new_req) 
ZEROMQ_CONSTRUCTOR(Reply, zsock_new_rep) 
ZEROMQ_CONSTRUCTOR(Dealer, zsock_new_dealer)
ZEROMQ_CONSTRUCTOR(Router, zsock_new_router)  
ZEROMQ_CONSTRUCTOR(Push, zsock_new_push) 
ZEROMQ_CONSTRUCTOR(Pull, zsock_new_pull) 
ZEROMQ_CONSTRUCTOR(XPublisher, zsock_new_xpub) 
ZEROMQ_CONSTRUCTOR(XSubscriber, zsock_new_xsub) 
ZEROMQ_CONSTRUCTOR(Pair, zsock_new_pair) 

/* -------------------------------------------------------
 * Socket veneer class 
 * ------------------------------------------------------- */

#define ZEROMQ_CONNECTMETHOD(name, connectfunc) value ZeroMQSocket_##name(vm *v, int nargs, value *args) { \
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args)); \
\
    if (nargs==1 && MORPHO_ISSTRING(MORPHO_GETARG(args, 0))) { \
        int ret=connectfunc(sock->socket, "%s", MORPHO_GETCSTRING(MORPHO_GETARG(args, 0))); \
        if (ret==-1) zeromq_error(v); \
    } \
\
    return MORPHO_NIL; \
}

ZEROMQ_CONNECTMETHOD(bind, zsock_bind)
ZEROMQ_CONNECTMETHOD(unbind, zsock_unbind)
ZEROMQ_CONNECTMETHOD(connect, zsock_connect)
ZEROMQ_CONNECTMETHOD(disconnect, zsock_disconnect)

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
MORPHO_METHOD(ZEROMQ_BIND_METHOD, ZeroMQSocket_bind, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_ENDPOINT_METHOD, ZeroMQSocket_endpoint, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_UNBIND_METHOD, ZeroMQSocket_unbind, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_CONNECT_METHOD, ZeroMQSocket_connect, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_DISCONNECT_METHOD, ZeroMQSocket_disconnect, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_SEND_METHOD, ZeroMQSocket_send, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_RECEIVE_METHOD, ZeroMQSocket_receive, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_SUBSCRIBE_METHOD, ZeroMQSocket_subscribe, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_UNSUBSCRIBE_METHOD, ZeroMQSocket_unsubscribe, BUILTIN_FLAGSEMPTY)
MORPHO_ENDCLASS

/* -------------------------------------------------------
 * Poller veneer class 
 * ------------------------------------------------------- */

/** Creates a new ZeroMQ poller */
objectzeromqpoller *object_newzeromqpoller(zpoller_t *poll) {
    objectzeromqpoller *new = (objectzeromqpoller *) object_new(sizeof(objectzeromqpoller), ZEROMQ_POLLER);
    if (new) {
        new->poller = poll; 
        dictionary_init(&new->readers);
    }
    return new; 
}

/** Add a socket to a poller */
void zeromqpoller_add(objectzeromqpoller *poll, value sock) {
    if (!ZEROMQ_ISSOCKET(sock)) return; 
    zsock_t *sockt = ZEROMQ_GETSOCKET(sock)->socket; 
    zpoller_add(poll->poller, sockt);
    dictionary_insert(&poll->readers, MORPHO_OBJECT(sockt), sock);
}

/** Remove a socket from a poller */
void zeromqpoller_remove(objectzeromqpoller *poll, value sock) {
    if (!ZEROMQ_ISSOCKET(sock)) return; 
    zsock_t *sockt = ZEROMQ_GETSOCKET(sock)->socket; 
    if (dictionary_get(&poll->readers, MORPHO_OBJECT(sockt), NULL)) {
        zpoller_remove(poll->poller, sockt);
    }
}

/** Constructor function for a ZMQ Poller object */
value ZeroMQPoller(vm *v, int nargs, value *args) { 
    value out = MORPHO_NIL; 

    zpoller_t *new = zpoller_new(NULL);
    if (new) {
        objectzeromqpoller *poll = object_newzeromqpoller(new);

        for (int i=0; i<nargs; i++) {
            zeromqpoller_add(poll, MORPHO_GETARG(args, i));
        }

        if (poll) out = MORPHO_OBJECT(poll);
        if (MORPHO_ISOBJECT(out)) morpho_bindobjects(v, 1, &out);
    }

    return out; 
} 

value ZeroMQPoller_wait(vm *v, int nargs, value *args) { 
    objectzeromqpoller *self = ZEROMQ_GETPOLLER(MORPHO_SELF(args));
    value out = MORPHO_NIL; 

    int wait = -1; // Indefinitely by default

    if (nargs==1) morpho_valuetoint(MORPHO_GETARG(args, 0), &wait);

    zsock_t *sock = zpoller_wait(self->poller, wait);
    if (sock) dictionary_get(&self->readers, MORPHO_OBJECT(sock), &out);

    return out; 
}

MORPHO_BEGINCLASS(ZeroMQPoller)
MORPHO_METHOD(ZEROMQ_WAIT_METHOD, ZeroMQPoller_wait, BUILTIN_FLAGSEMPTY)
MORPHO_ENDCLASS

/* -------------------------------------------------------
 * Proxy 
 * ------------------------------------------------------- */

/** Creates a new ZeroMQ proxy object */
objectzeromqproxy *object_newzeromqproxy(zactor_t *proxy) {
    objectzeromqproxy *new = (objectzeromqproxy *) object_new(sizeof(objectzeromqproxy), ZEROMQ_PROXY);
    if (new) new->proxy = proxy; 
    return new; 
}

/** Constructor function for a ZMQ Proxy object */
value ZeroMQProxy(vm *v, int nargs, value *args) { 
    value out = MORPHO_NIL; 

    zactor_t *new = zactor_new(zproxy, NULL);
    if (new) {
        objectzeromqproxy *poll = object_newzeromqproxy(new);

        if (poll) out = MORPHO_OBJECT(poll);
        if (MORPHO_ISOBJECT(out)) morpho_bindobjects(v, 1, &out);
    }

    return out; 
} 

value ZeroMQProxy_pause(vm *v, int nargs, value *args) { 
    objectzeromqproxy *self = ZEROMQ_GETPROXY(MORPHO_SELF(args));

    if (self->proxy) {
        zstr_sendx(self->proxy, "PAUSE", NULL);
        zsock_wait(self->proxy);
    }

    return MORPHO_NIL; 
}

value ZeroMQProxy_resume(vm *v, int nargs, value *args) { 
    objectzeromqproxy *self = ZEROMQ_GETPROXY(MORPHO_SELF(args));

    if (self->proxy) {
        zstr_sendx(self->proxy, "RESUME", NULL);
        zsock_wait(self->proxy);
    }

    return MORPHO_NIL; 
}

MORPHO_BEGINCLASS(ZeroMQProxy)
//MORPHO_METHOD(ZEROMQ_WAIT_METHOD, ZeroMQPoller_wait, BUILTIN_FLAGSEMPTY)
MORPHO_METHOD(ZEROMQ_PAUSE_METHOD, ZeroMQProxy_pause, BUILTIN_FLAGSEMPTY),
MORPHO_METHOD(ZEROMQ_RESUME_METHOD, ZeroMQProxy_resume, BUILTIN_FLAGSEMPTY) 
MORPHO_ENDCLASS

/* -------------------------------------------------------
 * Initialization and finalization
 * ------------------------------------------------------- */

void zeromq_initialize(void) { 
    objectzeromqsockettype=object_addtype(&objectzeromqsocketdefn);
    objectzeromqpollertype=object_addtype(&objectzeromqpollerdefn);
    
    objectstring objclassname = MORPHO_STATICSTRING(OBJECT_CLASSNAME);
    value objclass = builtin_findclass(MORPHO_OBJECT(&objclassname));
    
    value zeromqsocketclass=builtin_addclass(ZEROMQ_SOCKETCLASSNAME, MORPHO_GETCLASSDEFINITION(ZeroMQSocket), objclass);
    object_setveneerclass(ZEROMQ_SOCKET, zeromqsocketclass);

    builtin_addfunction(ZEROMQ_PUBLISHER_CONS, ZeroMQPublisher, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_SUBSCRIBER_CONS, ZeroMQSubscriber, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_REQUEST_CONS, ZeroMQRequest, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_REPLY_CONS, ZeroMQReply, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_DEALER_CONS, ZeroMQDealer, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_ROUTER_CONS, ZeroMQRouter, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_PUSH_CONS, ZeroMQPush, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_PULL_CONS, ZeroMQPull, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_XPUBLISHER_CONS, ZeroMQXPublisher, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_XSUBSCRIBER_CONS, ZeroMQXSubscriber, BUILTIN_FLAGSEMPTY);
    builtin_addfunction(ZEROMQ_PAIR_CONS, ZeroMQPair, BUILTIN_FLAGSEMPTY);

    // ZMQPoller
    value zeromqpollerclass=builtin_addclass(ZEROMQ_POLLERCLASSNAME, MORPHO_GETCLASSDEFINITION(ZeroMQPoller), objclass);
    object_setveneerclass(ZEROMQ_POLLER, zeromqpollerclass);

    builtin_addfunction(ZEROMQ_POLLERCLASSNAME, ZeroMQPoller, BUILTIN_FLAGSEMPTY);

    // ZMQProxy
    value zeromqproxyclass=builtin_addclass(ZEROMQ_PROXYCLASSNAME, MORPHO_GETCLASSDEFINITION(ZeroMQProxy), objclass);
    object_setveneerclass(ZEROMQ_PROXY, zeromqproxyclass);

    //builtin_addfunction(ZEROMQ_PROXYCLASSNAME, ZeroMQProxy, BUILTIN_FLAGSEMPTY);

    morpho_defineerror(ZEROMQ_CONSARGS, ERROR_HALT, ZEROMQ_CONSARGS_MSG);
    morpho_defineerror(ZEROMQ_ERR, ERROR_HALT, ZEROMQ_ERR_MSG);
}

void zeromq_finalize(void) { 
}
