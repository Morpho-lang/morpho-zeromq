#include <stdio.h>
#include <morpho/morpho.h>
#include <morpho/builtin.h>

#define ZEROMQ_SOCKETCLASSNAME    "ZMQSocket"
#define ZEROMQ_POLLERCLASSNAME    "ZMQPoller"
#define ZEROMQ_PROXYCLASSNAME     "ZMQProxy"

#define ZEROMQ_PUBLISHER_CONS     "ZMQPublisher"
#define ZEROMQ_SUBSCRIBER_CONS    "ZMQSubscriber"
#define ZEROMQ_REQUEST_CONS       "ZMQRequest"
#define ZEROMQ_REPLY_CONS         "ZMQReply"
#define ZEROMQ_DEALER_CONS        "ZMQDealer"
#define ZEROMQ_ROUTER_CONS        "ZMQRouter"
#define ZEROMQ_PUSH_CONS          "ZMQPush"
#define ZEROMQ_PULL_CONS          "ZMQPull"
#define ZEROMQ_XPUBLISHER_CONS    "ZMQXPublisher"
#define ZEROMQ_XSUBSCRIBER_CONS   "ZMQXSubscriber"
#define ZEROMQ_PAIR_CONS          "ZMQPair"

#define ZEROMQ_BIND_METHOD        "bind"
#define ZEROMQ_ENDPOINT_METHOD    "endpoint"
#define ZEROMQ_UNBIND_METHOD      "unbind"
#define ZEROMQ_CONNECT_METHOD     "connect"
#define ZEROMQ_DISCONNECT_METHOD  "disconnect"

#define ZEROMQ_SEND_METHOD        "send"
#define ZEROMQ_RECEIVE_METHOD     "receive"
#define ZEROMQ_WAIT_METHOD        "wait"
#define ZEROMQ_PAUSE_METHOD       "pause"
#define ZEROMQ_RESUME_METHOD      "resume"
#define ZEROMQ_ADD_METHOD         "add"
#define ZEROMQ_REMOVE_METHOD      "remove"

#define ZEROMQ_TIMEOUT_METHOD     "timeout"
#define ZEROMQ_LINGER_METHOD      "linger"
#define ZEROMQ_IDENTITY_METHOD    "identity"
#define ZEROMQ_UNBOUNDED_METHOD   "unbounded"

#define ZEROMQ_SUBSCRIBE_METHOD   "subscribe"
#define ZEROMQ_UNSUBSCRIBE_METHOD "unsubscribe"

#define ZEROMQ_ARGS               "ZMQArgs"
#define ZEROMQ_ARGS_MSG           "Incorrect arguments for ZeroMQ method."

#define ZEROMQ_ERR                "ZMQErr"
#define ZEROMQ_ERR_MSG            "ZeroMQ error: %s."

/** Common flag combinations for reviewed ZeroMQ builtins */
#define ZEROMQ_FN_CONS \
    (MORPHO_FN_CONSTRUCTOR|MORPHO_FN_ALLOCATES|MORPHO_FN_THROWS|MORPHO_FN_IO)
#define ZEROMQ_FN_IOMUT \
    (MORPHO_FN_MUTATES|MORPHO_FN_THROWS|MORPHO_FN_IO)
#define ZEROMQ_FN_MUT \
    (MORPHO_FN_MUTATES|MORPHO_FN_THROWS)
#define ZEROMQ_FN_RECV \
    (MORPHO_FN_ALLOCATES|MORPHO_FN_THROWS|MORPHO_FN_IO)
#define ZEROMQ_FN_GETSTR \
    (MORPHO_FN_ALLOCATES|MORPHO_FN_THROWS)
#define ZEROMQ_FN_WAIT \
    (MORPHO_FN_IO)

// Size needed to get a ZMQ type string
#define ZEROMQ_TYPEBUFFERLENGTH   64

void zeromq_initialize(void);
void zeromq_finalize(void);
