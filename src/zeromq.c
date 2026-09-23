#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef _WIN32
/* Morpho's import lib exports data as __imp_* only. lld-link will not
 * auto-import those without dllimport on the declarations. */
#define extern __declspec(dllimport) extern
#endif
#include <morpho.h>
#include <classes.h>
#include "zeromq.h"
#ifdef _WIN32
#undef extern
#endif

#include <czmq.h>

/* -------------------------------------------------------
 * ZeroMQ socket object type
 * ------------------------------------------------------- */

typedef struct {
    object obj;
    zsock_t *socket;
    bool bound; /* true while endpoint() should report zsock_endpoint */
} objectzeromqsocket;

objecttype objectzeromqsockettype;
#define ZEROMQ_SOCKET objectzeromqsockettype

/** Gets the object as a zeromq socket */
#define ZEROMQ_GETSOCKET(val)   ((objectzeromqsocket *) MORPHO_GETOBJECT(val))

/** Tests whether an object is a socket */
#define ZEROMQ_ISSOCKET(val) object_istype(val, ZEROMQ_SOCKET)

/** ZeroMQ socket definitions */
void objectzeromqsocket_printfn(object *obj, void *v) {
    objectzeromqsocket *d = (objectzeromqsocket *) obj;
    const char *type = zsock_type_str(d->socket);
    morpho_printf(v, "<%s: %s>", ZEROMQ_SOCKETCLASSNAME, type);
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
    .sizefn=objectzeromqsocket_sizefn,
    .hashfn=NULL,
    .cmpfn=NULL
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

/** Tests whether an object is a poller */
#define ZEROMQ_ISPOLLER(val) object_istype(val, ZEROMQ_POLLER)

/** Gets the object as a zeromq poller */
#define ZEROMQ_GETPOLLER(val)   ((objectzeromqpoller *) MORPHO_GETOBJECT(val))

/** ZeroMQ poller definitions */
void objectzeromqpoller_printfn(object *obj, void *v) {
    morpho_printf(v, "<ZeroMQPoller>");
}

void objectzeromqpoller_markfn(object *obj, void *v) {
    objectzeromqpoller *poller = (objectzeromqpoller *) obj;
    for (unsigned int i=0; i<poller->readers.capacity; i++) {
        dictionaryentry *e = &poller->readers.contents[i];
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
    .sizefn=objectzeromqpoller_sizefn,
    .hashfn=NULL,
    .cmpfn=NULL
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

/** Tests whether an object is a proxy */
#define ZEROMQ_ISPROXY(val) object_istype(val, ZEROMQ_PROXY)

/** Gets the object as a zeromq proxy */
#define ZEROMQ_GETPROXY(val)   ((objectzeromqproxy *) MORPHO_GETOBJECT(val))

/** ZeroMQ proxy definitions */
void objectzeromqproxy_printfn(object *obj, void *v) {
    morpho_printf(v, "<ZeroMQProxy>");
}

void objectzeromqproxy_markfn(object *obj, void *v) {
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
    .sizefn=objectzeromqproxy_sizefn,
    .hashfn=NULL,
    .cmpfn=NULL
};

/* -------------------------------------------------------
 * ZeroMQ errors
 * ------------------------------------------------------- */

/** Raises a catchable ZeroMQ error */
void zeromq_error(vm *v) {
    morpho_runtimeerror(v, ZEROMQ_ERR, zmq_strerror(zmq_errno()));
}

/** True if a failed receive is a timeout / would-block, not a hard error */
bool zeromq_isreceiveempty(void) {
    int err = zmq_errno();
    return (err == EAGAIN || err == ETIMEDOUT || zsys_interrupted);
}

/** Bind a Morpho string value created by object_stringfromcstring */
value zeromq_bindstring(vm *v, value str) {
    return morpho_wrapandbind(v, MORPHO_ISOBJECT(str) ? MORPHO_GETOBJECT(str) : NULL);
}

/* -------------------------------------------------------
 * Constructor functions
 * ------------------------------------------------------- */

/** Creates a new ZeroMQ socket */
objectzeromqsocket *object_newzeromqsocket(zsock_t *socket) {
    objectzeromqsocket *new = (objectzeromqsocket *) object_new(sizeof(objectzeromqsocket), ZEROMQ_SOCKET);
    if (new) {
        new->socket = socket;
        /* CZMQ caches the last bind endpoint even after unbind; track bind state ourselves */
        new->bound = (socket && zsock_endpoint(socket)!=NULL);
    }
    return new;
}

/** Wrap a CZMQ socket as a Morpho object, destroying the socket on allocation failure */
value zeromq_wrapsocket(vm *v, zsock_t *sock) {
    if (!sock) {
        zeromq_error(v);
        return MORPHO_NIL;
    }
    value out = morpho_wrapandbind(v, (object *) object_newzeromqsocket(sock));
    if (MORPHO_ISNIL(out)) zsock_destroy(&sock);
    return out;
}

/** Generic constructor function */
#define ZEROMQ_CONSTRUCTOR(name, consfunc) value ZeroMQ##name (vm *v, int nargs, value *args) { \
    char *endpoint = NULL; \
    if (nargs==1) endpoint = MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)); \
    return zeromq_wrapsocket(v, consfunc(endpoint)); \
}

ZEROMQ_CONSTRUCTOR(Publisher, zsock_new_pub)

value ZeroMQSubscriber(vm *v, int nargs, value *args) {
    char *subs = "";
    char *endpoint = NULL;

    if (nargs>=1) endpoint = MORPHO_GETCSTRING(MORPHO_GETARG(args, 0));
    if (nargs==2) subs = MORPHO_GETCSTRING(MORPHO_GETARG(args, 1));

    return zeromq_wrapsocket(v, zsock_new_sub(endpoint, subs));
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

value ZeroMQSocket_bind(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    int ret = zsock_bind(sock->socket, "%s", MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    if (ret==-1) zeromq_error(v);
    else sock->bound = true;
    return MORPHO_NIL;
}

value ZeroMQSocket_connect(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    int ret = zsock_connect(sock->socket, "%s", MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    if (ret==-1) zeromq_error(v);
    return MORPHO_NIL;
}

value ZeroMQSocket_disconnect(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    int ret = zsock_disconnect(sock->socket, "%s", MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    if (ret==-1) zeromq_error(v);
    return MORPHO_NIL;
}

/** Unbind the socket's current endpoint; no-op if not bound */
value ZeroMQSocket_unbind(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    if (!sock->bound) return MORPHO_NIL;

    const char *ep = zsock_endpoint(sock->socket);
    if (!ep) {
        sock->bound = false;
        return MORPHO_NIL;
    }

    int ret = zsock_unbind(sock->socket, "%s", ep);
    if (ret==-1) zeromq_error(v);
    else sock->bound = false;
    return MORPHO_NIL;
}

/** Unbind from a specified endpoint */
value ZeroMQSocket_unbind__string(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    int ret = zsock_unbind(sock->socket, "%s", MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    if (ret==-1) zeromq_error(v);
    else sock->bound = false;
    return MORPHO_NIL;
}

/** Determine the endpoint */
value ZeroMQSocket_endpoint(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    if (!sock->bound) return MORPHO_NIL;
    const char *ep = zsock_endpoint(sock->socket);
    if (!ep) return MORPHO_NIL;
    return zeromq_bindstring(v, object_stringfromcstring(ep, strlen(ep)));
}

/** Send frames from a list of string values */
bool zeromq_sendframes(vm *v, zsock_t *sock, unsigned int n, value *frames) {
    zmsg_t *msg = zmsg_new();
    if (!msg) {
        morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED);
        return false;
    }

    for (unsigned int i=0; i<n; i++) {
        if (!MORPHO_ISSTRING(frames[i])) {
            zmsg_destroy(&msg);
            morpho_runtimeerror(v, ZEROMQ_ARGS);
            return false;
        }
        if (zmsg_addstr(msg, MORPHO_GETCSTRING(frames[i]))==-1) {
            zmsg_destroy(&msg);
            zeromq_error(v);
            return false;
        }
    }

    if (zmsg_send(&msg, sock)==-1) {
        zmsg_destroy(&msg);
        zeromq_error(v);
        return false;
    }
    return true;
}

/** Send a single-frame string message */
value ZeroMQSocket_send__string(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    int ret = zstr_send(sock->socket, MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    if (ret==-1) zeromq_error(v);
    return MORPHO_NIL;
}

/** Send a multipart message from a Tuple of strings */
value ZeroMQSocket_send__tuple(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    objecttuple *tuple = MORPHO_GETTUPLE(MORPHO_GETARG(args, 0));
    zeromq_sendframes(v, sock->socket, tuple->length, tuple->tuple);
    return MORPHO_NIL;
}

/** Send a multipart message from a List of strings */
value ZeroMQSocket_send__list(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    objectlist *list = MORPHO_GETLIST(MORPHO_GETARG(args, 0));
    zeromq_sendframes(v, sock->socket, list_length(list), list->val.data);
    return MORPHO_NIL;
}

/** Receive */
value ZeroMQSocket_receive(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));

    zmsg_t *msg = zmsg_recv(sock->socket);
    if (!msg) {
        if (!zeromq_isreceiveempty()) zeromq_error(v);
        return MORPHO_NIL;
    }

    size_t n = zmsg_size(msg);
    if (n==0) {
        zmsg_destroy(&msg);
        return MORPHO_NIL;
    }

    if (n==1) {
        char *string = zmsg_popstr(msg);
        zmsg_destroy(&msg);
        if (!string) {
            if (!zeromq_isreceiveempty()) zeromq_error(v);
            return MORPHO_NIL;
        }
        value out = object_stringfromcstring(string, strlen(string));
        zstr_free(&string);
        return zeromq_bindstring(v, out);
    }

    value *frames = MORPHO_MALLOC(n * sizeof(value));
    if (!frames) {
        zmsg_destroy(&msg);
        morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED);
        return MORPHO_NIL;
    }

    for (size_t i=0; i<n; i++) {
        char *string = zmsg_popstr(msg);
        if (!string) {
            for (size_t j=0; j<i; j++) {
                if (MORPHO_ISOBJECT(frames[j])) object_free(MORPHO_GETOBJECT(frames[j]));
            }
            MORPHO_FREE(frames);
            zmsg_destroy(&msg);
            morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED);
            return MORPHO_NIL;
        }
        frames[i] = object_stringfromcstring(string, strlen(string));
        zstr_free(&string);
        if (!MORPHO_ISOBJECT(frames[i])) {
            for (size_t j=0; j<i; j++) {
                if (MORPHO_ISOBJECT(frames[j])) object_free(MORPHO_GETOBJECT(frames[j]));
            }
            MORPHO_FREE(frames);
            zmsg_destroy(&msg);
            morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED);
            return MORPHO_NIL;
        }
    }
    zmsg_destroy(&msg);

    objecttuple *tuple = object_newtuple((unsigned int) n, frames);
    if (!tuple) {
        for (size_t j=0; j<n; j++) {
            if (MORPHO_ISOBJECT(frames[j])) object_free(MORPHO_GETOBJECT(frames[j]));
        }
        MORPHO_FREE(frames);
        morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED);
        return MORPHO_NIL;
    }
    MORPHO_FREE(frames);
    return morpho_wrapandbindrecursive(v, (object *) tuple);
}

/** Subscribe */
value ZeroMQSocket_subscribe(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    zsock_set_subscribe(sock->socket, MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    return MORPHO_NIL;
}

/** Unsubscribe */
value ZeroMQSocket_unsubscribe(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    zsock_set_unsubscribe(sock->socket, MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    return MORPHO_NIL;
}

/** Get receive timeout in milliseconds */
value ZeroMQSocket_timeout(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    return MORPHO_INTEGER(zsock_rcvtimeo(sock->socket));
}

/** Set receive timeout in milliseconds */
value ZeroMQSocket_timeout__int(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    zsock_set_rcvtimeo(sock->socket, MORPHO_GETINTEGERVALUE(MORPHO_GETARG(args, 0)));
    return MORPHO_NIL;
}

value ZeroMQSocket_timeout__float(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    zsock_set_rcvtimeo(sock->socket, (int) MORPHO_GETFLOATVALUE(MORPHO_GETARG(args, 0)));
    return MORPHO_NIL;
}

/** Get linger period in milliseconds */
value ZeroMQSocket_linger(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    return MORPHO_INTEGER(zsock_linger(sock->socket));
}

/** Set linger period in milliseconds */
value ZeroMQSocket_linger__int(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    zsock_set_linger(sock->socket, MORPHO_GETINTEGERVALUE(MORPHO_GETARG(args, 0)));
    return MORPHO_NIL;
}

value ZeroMQSocket_linger__float(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    zsock_set_linger(sock->socket, (int) MORPHO_GETFLOATVALUE(MORPHO_GETARG(args, 0)));
    return MORPHO_NIL;
}

/** Get socket identity */
value ZeroMQSocket_identity(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    char *id = zsock_identity(sock->socket);
    if (!id) return MORPHO_NIL;
    value out = object_stringfromcstring(id, strlen(id));
    zstr_free(&id);
    return zeromq_bindstring(v, out);
}

/** Set socket identity */
value ZeroMQSocket_identity__string(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    zsock_set_identity(sock->socket, MORPHO_GETCSTRING(MORPHO_GETARG(args, 0)));
    return MORPHO_NIL;
}

/** Set infinite high-water marks */
value ZeroMQSocket_unbounded(vm *v, int nargs, value *args) {
    objectzeromqsocket *sock = ZEROMQ_GETSOCKET(MORPHO_SELF(args));
    zsock_set_unbounded(sock->socket);
    return MORPHO_NIL;
}

MORPHO_BEGINCLASS(ZeroMQSocket)
MORPHO_METHOD_SIGNATURE(ZEROMQ_BIND_METHOD, "Nil (String)", ZeroMQSocket_bind, ZEROMQ_FN_IOMUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_UNBIND_METHOD, "Nil ()", ZeroMQSocket_unbind, ZEROMQ_FN_IOMUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_UNBIND_METHOD, "Nil (String)", ZeroMQSocket_unbind__string, ZEROMQ_FN_IOMUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_CONNECT_METHOD, "Nil (String)", ZeroMQSocket_connect, ZEROMQ_FN_IOMUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_DISCONNECT_METHOD, "Nil (String)", ZeroMQSocket_disconnect, ZEROMQ_FN_IOMUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_ENDPOINT_METHOD, " ()", ZeroMQSocket_endpoint, ZEROMQ_FN_GETSTR),
MORPHO_METHOD_SIGNATURE(ZEROMQ_SEND_METHOD, "Nil (String)", ZeroMQSocket_send__string, ZEROMQ_FN_IOMUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_SEND_METHOD, "Nil (Tuple)", ZeroMQSocket_send__tuple, ZEROMQ_FN_IOMUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_SEND_METHOD, "Nil (List)", ZeroMQSocket_send__list, ZEROMQ_FN_IOMUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_RECEIVE_METHOD, " ()", ZeroMQSocket_receive, ZEROMQ_FN_RECV),
MORPHO_METHOD_SIGNATURE(ZEROMQ_SUBSCRIBE_METHOD, "Nil (String)", ZeroMQSocket_subscribe, ZEROMQ_FN_MUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_UNSUBSCRIBE_METHOD, "Nil (String)", ZeroMQSocket_unsubscribe, ZEROMQ_FN_MUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_TIMEOUT_METHOD, "Int ()", ZeroMQSocket_timeout, MORPHO_FN_NONE),
MORPHO_METHOD_SIGNATURE(ZEROMQ_TIMEOUT_METHOD, "Nil (Int)", ZeroMQSocket_timeout__int, ZEROMQ_FN_MUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_TIMEOUT_METHOD, "Nil (Float)", ZeroMQSocket_timeout__float, ZEROMQ_FN_MUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_LINGER_METHOD, "Int ()", ZeroMQSocket_linger, MORPHO_FN_NONE),
MORPHO_METHOD_SIGNATURE(ZEROMQ_LINGER_METHOD, "Nil (Int)", ZeroMQSocket_linger__int, ZEROMQ_FN_MUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_LINGER_METHOD, "Nil (Float)", ZeroMQSocket_linger__float, ZEROMQ_FN_MUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_IDENTITY_METHOD, " ()", ZeroMQSocket_identity, ZEROMQ_FN_GETSTR),
MORPHO_METHOD_SIGNATURE(ZEROMQ_IDENTITY_METHOD, "Nil (String)", ZeroMQSocket_identity__string, ZEROMQ_FN_MUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_UNBOUNDED_METHOD, "Nil ()", ZeroMQSocket_unbounded, ZEROMQ_FN_MUT)
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
bool zeromqpoller_add(objectzeromqpoller *poll, value sock) {
    if (!ZEROMQ_ISSOCKET(sock)) return false;
    zsock_t *sockt = ZEROMQ_GETSOCKET(sock)->socket;
    zpoller_add(poll->poller, sockt);
    dictionary_insert(&poll->readers, sock, sock);
    return true;
}

/** Remove a socket from a poller */
bool zeromqpoller_remove(objectzeromqpoller *poll, value sock) {
    if (!ZEROMQ_ISSOCKET(sock)) return false;
    zsock_t *sockt = ZEROMQ_GETSOCKET(sock)->socket;
    if (dictionary_get(&poll->readers, sock, NULL)) {
        zpoller_remove(poll->poller, sockt);
        dictionary_remove(&poll->readers, sock);
    }
    return true;
}

/** Find the Morpho socket corresponding to a czmq socket */
value zeromqpoller_findsocket(objectzeromqpoller *poll, zsock_t *sockt) {
    if (!sockt) return MORPHO_NIL;

    for (unsigned int i=0; i<poll->readers.capacity; i++) {
        dictionaryentry *e = &poll->readers.contents[i];
        if (MORPHO_ISNIL(e->key) || !ZEROMQ_ISSOCKET(e->val)) continue;
        if (ZEROMQ_GETSOCKET(e->val)->socket == sockt) return e->val;
    }

    return MORPHO_NIL;
}

/** Constructor function for a ZMQ Poller object */
value ZeroMQPoller(vm *v, int nargs, value *args) {
    zpoller_t *new = zpoller_new(NULL);
    if (!new) {
        zeromq_error(v);
        return MORPHO_NIL;
    }

    objectzeromqpoller *poll = object_newzeromqpoller(new);
    if (!poll) {
        zpoller_destroy(&new);
        morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED);
        return MORPHO_NIL;
    }

    for (int i=0; i<nargs; i++) {
        if (!zeromqpoller_add(poll, MORPHO_GETARG(args, i))) {
            object_free((object *) poll);
            morpho_runtimeerror(v, ZEROMQ_ARGS);
            return MORPHO_NIL;
        }
    }

    return morpho_wrapandbind(v, (object *) poll);
}

value ZeroMQPoller_wait(vm *v, int nargs, value *args) {
    objectzeromqpoller *self = ZEROMQ_GETPOLLER(MORPHO_SELF(args));
    zsock_t *sock = zpoller_wait(self->poller, -1);
    if (sock) return zeromqpoller_findsocket(self, sock);
    return MORPHO_NIL;
}

value ZeroMQPoller_wait__int(vm *v, int nargs, value *args) {
    objectzeromqpoller *self = ZEROMQ_GETPOLLER(MORPHO_SELF(args));
    int wait = MORPHO_GETINTEGERVALUE(MORPHO_GETARG(args, 0));
    zsock_t *sock = zpoller_wait(self->poller, wait);
    if (sock) return zeromqpoller_findsocket(self, sock);
    return MORPHO_NIL;
}

value ZeroMQPoller_wait__float(vm *v, int nargs, value *args) {
    objectzeromqpoller *self = ZEROMQ_GETPOLLER(MORPHO_SELF(args));
    int wait = (int) MORPHO_GETFLOATVALUE(MORPHO_GETARG(args, 0));
    zsock_t *sock = zpoller_wait(self->poller, wait);
    if (sock) return zeromqpoller_findsocket(self, sock);
    return MORPHO_NIL;
}

value ZeroMQPoller_add(vm *v, int nargs, value *args) {
    objectzeromqpoller *self = ZEROMQ_GETPOLLER(MORPHO_SELF(args));
    if (!zeromqpoller_add(self, MORPHO_GETARG(args, 0))) morpho_runtimeerror(v, ZEROMQ_ARGS);
    return MORPHO_NIL;
}

value ZeroMQPoller_remove(vm *v, int nargs, value *args) {
    objectzeromqpoller *self = ZEROMQ_GETPOLLER(MORPHO_SELF(args));
    if (!zeromqpoller_remove(self, MORPHO_GETARG(args, 0))) morpho_runtimeerror(v, ZEROMQ_ARGS);
    return MORPHO_NIL;
}

MORPHO_BEGINCLASS(ZeroMQPoller)
MORPHO_METHOD_SIGNATURE(ZEROMQ_WAIT_METHOD, " ()", ZeroMQPoller_wait, ZEROMQ_FN_WAIT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_WAIT_METHOD, " (Int)", ZeroMQPoller_wait__int, ZEROMQ_FN_WAIT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_WAIT_METHOD, " (Float)", ZeroMQPoller_wait__float, ZEROMQ_FN_WAIT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_ADD_METHOD, "Nil (ZMQSocket)", ZeroMQPoller_add, ZEROMQ_FN_MUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_REMOVE_METHOD, "Nil (ZMQSocket)", ZeroMQPoller_remove, ZEROMQ_FN_MUT)
MORPHO_ENDCLASS

/* -------------------------------------------------------
 * Proxy
 * ------------------------------------------------------- */

/** Map a Morpho ZMQ* constructor to a czmq zsock_type_str name (PUB, XSUB, …) */
bool zeromq_sockettypefromfn(value fn, char *out) {
    if (!out) return false;

    value name = MORPHO_NIL;
    if (MORPHO_ISBUILTINFUNCTION(fn)) {
        name = MORPHO_GETBUILTINFUNCTION(fn)->name;
    } else if (MORPHO_ISMETAFUNCTION(fn)) {
        name = MORPHO_GETMETAFUNCTION(fn)->name;
    } else return false;

    if (!MORPHO_ISSTRING(name)) return false;
    char *fnname = MORPHO_GETCSTRING(name);
    if (strncmp(fnname, "ZMQ", 3)!=0) return false;

    /* Keep full Morpho names for users; translate to CZMQ short type strings. */
    static const struct { const char *morpho; const char *czmq; } map[] = {
        { "Publisher",   "PUB" },
        { "Subscriber",  "SUB" },
        { "Request",     "REQ" },
        { "Reply",       "REP" },
        { "Dealer",      "DEALER" },
        { "Router",      "ROUTER" },
        { "Push",        "PUSH" },
        { "Pull",        "PULL" },
        { "XPublisher",  "XPUB" },
        { "XSubscriber", "XSUB" },
        { "Pair",        "PAIR" },
        { NULL, NULL }
    };

    const char *suffix = fnname + 3;
    for (int i=0; map[i].morpho; i++) {
        if (strcmp(suffix, map[i].morpho)==0) {
            strcpy(out, map[i].czmq);
            return true;
        }
    }
    return false;
}

/** Send a command to the proxy actor and wait for its reply */
bool zeromq_proxycmd(vm *v, zactor_t *proxy, const char *cmd, const char *a, const char *b) {
    int sent = (b) ? zstr_sendx(proxy, cmd, a, b, NULL)
                   : (a) ? zstr_sendx(proxy, cmd, a, NULL)
                         : zstr_sendx(proxy, cmd, NULL);
    if (sent==-1) {
        zeromq_error(v);
        return false;
    }
    if (zsock_wait(proxy)==-1) {
        zeromq_error(v);
        return false;
    }
    return true;
}

/** Creates a new ZeroMQ proxy object */
objectzeromqproxy *object_newzeromqproxy(zactor_t *proxy) {
    objectzeromqproxy *new = (objectzeromqproxy *) object_new(sizeof(objectzeromqproxy), ZEROMQ_PROXY);
    if (new) new->proxy = proxy;
    return new;
}

/** Constructor: ZMQProxy(frontType, frontEndpoint, backType, backEndpoint) */
value ZeroMQProxy(vm *v, int nargs, value *args) {
    char ftype[ZEROMQ_TYPEBUFFERLENGTH];
    char btype[ZEROMQ_TYPEBUFFERLENGTH];

    if (!zeromq_sockettypefromfn(MORPHO_GETARG(args, 0), ftype) ||
        !zeromq_sockettypefromfn(MORPHO_GETARG(args, 2), btype)) {
        morpho_runtimeerror(v, ZEROMQ_ARGS);
        return MORPHO_NIL;
    }

    zactor_t *actor = zactor_new(zproxy, NULL);
    if (!actor) {
        zeromq_error(v);
        return MORPHO_NIL;
    }

    const char *fep = MORPHO_GETCSTRING(MORPHO_GETARG(args, 1));
    const char *bep = MORPHO_GETCSTRING(MORPHO_GETARG(args, 3));

    if (!zeromq_proxycmd(v, actor, "FRONTEND", ftype, fep) ||
        !zeromq_proxycmd(v, actor, "BACKEND", btype, bep)) {
        zactor_destroy(&actor);
        return MORPHO_NIL;
    }

    objectzeromqproxy *proxy = object_newzeromqproxy(actor);
    if (!proxy) {
        zactor_destroy(&actor);
        morpho_runtimeerror(v, ERROR_ALLOCATIONFAILED);
        return MORPHO_NIL;
    }

    value out = morpho_wrapandbind(v, (object *) proxy);
    if (MORPHO_ISNIL(out)) object_free((object *) proxy);
    return out;
}

value ZeroMQProxy_pause(vm *v, int nargs, value *args) {
    objectzeromqproxy *self = ZEROMQ_GETPROXY(MORPHO_SELF(args));
    if (self->proxy) zeromq_proxycmd(v, self->proxy, "PAUSE", NULL, NULL);
    return MORPHO_NIL;
}

value ZeroMQProxy_resume(vm *v, int nargs, value *args) {
    objectzeromqproxy *self = ZEROMQ_GETPROXY(MORPHO_SELF(args));
    if (self->proxy) zeromq_proxycmd(v, self->proxy, "RESUME", NULL, NULL);
    return MORPHO_NIL;
}

MORPHO_BEGINCLASS(ZeroMQProxy)
MORPHO_METHOD_SIGNATURE(ZEROMQ_PAUSE_METHOD, "Nil ()", ZeroMQProxy_pause, ZEROMQ_FN_IOMUT),
MORPHO_METHOD_SIGNATURE(ZEROMQ_RESUME_METHOD, "Nil ()", ZeroMQProxy_resume, ZEROMQ_FN_IOMUT)
MORPHO_ENDCLASS

/* -------------------------------------------------------
 * Initialization and finalization
 * ------------------------------------------------------- */

/** Register a socket constructor with no-arg and String overloads */
void zeromq_addsocketconstructor(char *name, builtinfunction fn) {
    morpho_addfunction(name, ZEROMQ_SOCKETCLASSNAME " ()", fn, ZEROMQ_FN_CONS, NULL);
    morpho_addfunction(name, ZEROMQ_SOCKETCLASSNAME " (String)", fn, ZEROMQ_FN_CONS, NULL);
}

MORPHO_EXPORT void zeromq_initialize(void) {
    objectzeromqsockettype=object_addtype(&objectzeromqsocketdefn);
    objectzeromqpollertype=object_addtype(&objectzeromqpollerdefn);
    objectzeromqproxytype=object_addtype(&objectzeromqproxydefn);

    objectstring objclassname = MORPHO_STATICSTRING(OBJECT_CLASSNAME);
    value objclass = builtin_findclass(MORPHO_OBJECT(&objclassname));

    value zeromqsocketclass=builtin_addclass(ZEROMQ_SOCKETCLASSNAME, MORPHO_GETCLASSDEFINITION(ZeroMQSocket), objclass);
    object_setveneerclass(ZEROMQ_SOCKET, zeromqsocketclass);

    zeromq_addsocketconstructor(ZEROMQ_PUBLISHER_CONS, ZeroMQPublisher);
    morpho_addfunction(ZEROMQ_SUBSCRIBER_CONS, ZEROMQ_SOCKETCLASSNAME " ()", ZeroMQSubscriber, ZEROMQ_FN_CONS, NULL);
    morpho_addfunction(ZEROMQ_SUBSCRIBER_CONS, ZEROMQ_SOCKETCLASSNAME " (String)", ZeroMQSubscriber, ZEROMQ_FN_CONS, NULL);
    morpho_addfunction(ZEROMQ_SUBSCRIBER_CONS, ZEROMQ_SOCKETCLASSNAME " (String, String)", ZeroMQSubscriber, ZEROMQ_FN_CONS, NULL);
    zeromq_addsocketconstructor(ZEROMQ_REQUEST_CONS, ZeroMQRequest);
    zeromq_addsocketconstructor(ZEROMQ_REPLY_CONS, ZeroMQReply);
    zeromq_addsocketconstructor(ZEROMQ_DEALER_CONS, ZeroMQDealer);
    zeromq_addsocketconstructor(ZEROMQ_ROUTER_CONS, ZeroMQRouter);
    zeromq_addsocketconstructor(ZEROMQ_PUSH_CONS, ZeroMQPush);
    zeromq_addsocketconstructor(ZEROMQ_PULL_CONS, ZeroMQPull);
    zeromq_addsocketconstructor(ZEROMQ_XPUBLISHER_CONS, ZeroMQXPublisher);
    zeromq_addsocketconstructor(ZEROMQ_XSUBSCRIBER_CONS, ZeroMQXSubscriber);
    zeromq_addsocketconstructor(ZEROMQ_PAIR_CONS, ZeroMQPair);

    value zeromqpollerclass=builtin_addclass(ZEROMQ_POLLERCLASSNAME, MORPHO_GETCLASSDEFINITION(ZeroMQPoller), objclass);
    object_setveneerclass(ZEROMQ_POLLER, zeromqpollerclass);
    morpho_addfunction(ZEROMQ_POLLERCLASSNAME, ZEROMQ_POLLERCLASSNAME " (...)", ZeroMQPoller, ZEROMQ_FN_CONS, NULL);

    value zeromqproxyclass=builtin_addclass(ZEROMQ_PROXYCLASSNAME, MORPHO_GETCLASSDEFINITION(ZeroMQProxy), objclass);
    object_setveneerclass(ZEROMQ_PROXY, zeromqproxyclass);
    morpho_addfunction(ZEROMQ_PROXYCLASSNAME, ZEROMQ_PROXYCLASSNAME " (_, String, _, String)", ZeroMQProxy, ZEROMQ_FN_CONS, NULL);

    morpho_defineerror(ZEROMQ_ARGS, ERROR_HALT, ZEROMQ_ARGS_MSG);
    morpho_defineerror(ZEROMQ_ERR, ERROR_USER, ZEROMQ_ERR_MSG);
}

MORPHO_EXPORT void zeromq_finalize(void) {
}
