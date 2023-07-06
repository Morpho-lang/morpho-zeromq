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
#define ZEROMQ_XSUBSCRIBER_CONS   "ZMQXSubcriber"
#define ZEROMQ_PAIR_CONS          "ZMQPair"

#define ZEROMQ_BIND_METHOD        "bind"
#define ZEROMQ_ENDPOINT_METHOD    "endpoint"
#define ZEROMQ_UNBIND_METHOD      "unbind"
#define ZEROMQ_CONNECT_METHOD     "connect"
#define ZEROMQ_DISCONNECT_METHOD  "disconnect"

#define ZEROMQ_SEND_METHOD        "send"
#define ZEROMQ_RECEIVE_METHOD     "receive"
#define ZEROMQ_SETUNBOUNDED_METHOD "setunbounded"
#define ZEROMQ_SIGNAL_METHOD      "signal"
#define ZEROMQ_WAIT_METHOD        "wait"
#define ZEROMQ_FLUSH_METHOD       "flush"
#define ZEROMQ_PAUSE_METHOD       "pause"
#define ZEROMQ_RESUME_METHOD      "resume"

#define ZEROMQ_SUBSCRIBE_METHOD   "subscribe"
#define ZEROMQ_UNSUBSCRIBE_METHOD "unsubscribe"

#define ZEROMQ_CONSARGS           "ZMQConsArgs"
#define ZEROMQ_CONSARGS_MSG       "Socket constructor argument should be an endpoint."

#define ZEROMQ_ERR                "ZMQErr"
#define ZEROMQ_ERR_MSG            "ZeroMQ error: %s."

void zeromq_initialize(void);