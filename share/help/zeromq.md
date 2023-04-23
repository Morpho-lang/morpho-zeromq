[comment]: # (ZeroMQ help)
[version]: # (0.6)

# ZeroMQ
[tagZeroMQ]: # (ZeroMQ)
[tagZMQ]: # (ZMQ)
[tag0MQ]: # (0MQ)

The `ZeroMQ` extension provides support for distributed computing through the `ZeroMQ` library. The API provided is designed to be high-level, at least relative to other language bindings, and follows the `czmq` patterns rather than the low-level `zmq` patterns. To use the `ZeroMQ` extension import it,

    import zeromq

You then create appropriate `ZMQSocket` objects and bind or connect to them by calling the appropriate constructor. `ZeroMQ` supports multiple types of transport, including interprocess, tcp, etc.; see the examples or `czmq`/`ZeroMQ` documentation for further details. See `ZMQSocket` for general information about sockets. 

Once appropriate sockets have been created, use the `send` and `receive` methods to send information. 

[showsubtopics]: # (subtopics)

## ZMQSocket
[tagsocket]: # (socket)
[tagzmqsocket]: # (ZMQSocket)

The `ZMQSocket` class provides access to `ZeroMQ` sockets, which abstract many kinds of inter-process and inter-machine transports into a single convenient API. `ZeroMQ` provides several *patterns*, reusable kinds of socket, that each have slightly different behavior and are designed to solve recurring problems in network architecture. 

To create a socket, use the appropriate constructor, e.g. 

    var requester = ZMQRequest("tcp://localhost:5555")

which connects to a TCP connection on port 5555. Sockets either *bind* (create) an endpoint, or *connect* (join) a specified endpoint. Generally, "stable" parts of your architecture, those that are persistent, should bind to an endpoint and "dynamic" parts should connect.

Some examples of different transports. 

    var rep = ZMQReply("tcp://*:5555") // Bind to a specified port an any available interface. 
    var rep = ZMQReply("tcp://127.0.0.1:5555") // Bind to a specified port on local loopback interface. 

    var req = ZMQRequest("tcp://localhost:5555") // Connect to a specified local port 
    var req = ZMQRequest("ipc:///tmp/feeds/0") // Connect to inter-process communication on UNIX only
    var req = ZMQRequest("inproc://endpoint-string") // Connect to an in-process communication socket.

Each socket type has a default action, either bind or connect. To override, simply create the socket without and endpoint specified and use the `bind` or `connect` methods: 

    var req = ZMQRequest() 
    req.bind("tcp://*:5555") 

For further information, consult the `czmq` documentation. 

## Request-Reply
[tagZMQrequest]: # (ZMQrequest)
[tagZMQreply]: # (ZMQreply)
[tagrequest]: # (request)
[tagreply]: # (reply)

The Request/Reply message exchange pattern implements a remote procedure call. It involves a client, who creates a `ZMQRequest` object, and a server, who creates a `ZMQReply` object. The client sends a message to the server, which receives and processes the request and returns a message in response. This sequence of request/reply continues as long as necessary, but in strict order. Messages sent out of order are ignored. 

Minimal client and server communicating via interprocess communication:

    var req = ZMQRequest("inproc://endpoint") 
    var rep = ZMQReply("inproc://endpoint") 

    req.send("Hello") 
    print rep.receive() 
    rep.send("World!") 
    print req.receive() 

By default, `ZMQRequest` *connects* to an endpoint, while `ZMQReply` *binds* to the endpoint. 

## Publisher-Subscriber
[tagpublisher]: # (publisher)
[tagsubscriber]: # (subscriber)

The Publisher/subscriber pattern is used for data distribution. It involves a publisher, who creates a `ZMQPublisher` object and broadcasts messages, and any number of subscribers, who each create a `ZMQSubscriber` object and receive messages.

By default, `ZMQPublisher` *binds* to an endpoint, and `ZMQSubscriber` *connects*. 

## Push-Pull
[tagpush]: # (push)
[tagpull]: # (pull)

The push/pull message exhange pattern is used for parallel task distribution. It involves two kinds of socket: `ZMQPush` used to send messages and `ZMQPull` used to receive messages. Both sockets can send and receive from multiple servers and clients. Messages are fair-queued into Pull sockets and load-balanced from Push sockets. 

By default, `ZMQPush` *connects* to an endpoint, and `ZMQPull` *binds*. 

## Dealer-Router

Provides a non-blocking request-reply pattern implemented by the `ZMQDealer` and `ZMQRouter` classes. 

By default, `ZMQRouter` *binds* to an endpoint, and `ZMQDealer` *connects*. 