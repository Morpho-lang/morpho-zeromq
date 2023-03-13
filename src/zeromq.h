#include <stdio.h>
#include <morpho/morpho.h>
#include <morpho/builtin.h>

#define ZEROMQ_SOCKETCLASSNAME    "ZeroMQSocket"

#define ZEROMQ_PUSH_CONS          "ZeroMQPush"
#define ZEROMQ_PULL_CONS          "ZeroMQPull"
#define ZEROMQ_REQUEST_CONS       "ZeroMQRequest"
#define ZEROMQ_REPLY_CONS         "ZeroMQReply"
#define ZEROMQ_DEALER_CONS        "ZeroMQDealer"
#define ZEROMQ_ROUTER_CONS        "ZeroMQRouter"
#define ZEROMQ_PUBLISH_CONS       "ZeroMQPublish"
#define ZEROMQ_SUBSCRIBE_CONS     "ZeroMQSubscribe"

#define ZEROMQ_SEND_METHOD        "send"
#define ZEROMQ_RECEIVE_METHOD     "receive"
#define ZEROMQ_ENDPOINT_METHOD    "endpoint"
#define ZEROMQ_SUBSCRIBE_METHOD   "subscribe"
#define ZEROMQ_UNSUBSCRIBE_METHOD "unsubscribe"

#define ZEROMQ_CONSARGS           "ZMQConsArgs"
#define ZEROMQ_CONSARGS_MSG       "Socket constructor requires an endpoint as an argument."

#define ZEROMQ_ERR                "ZMQErr"
#define ZEROMQ_ERR_MSG            "ZeroMQ error: %s."

void zeromq_initialize(void);