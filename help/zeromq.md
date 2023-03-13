[comment]: # (ZeroMQ help)
[version]: # (0.6)

# ZeroMQ
[tagzeromq]: # (zeromq)

The `zeromq` extension provides support for distributed computing through the `ZeroMQ` library. To use it,

    import zeromq 

[showsubtopics]: # (subtopics)

## Request-Reply
[tagzeromqrequest]: # (zeromqrequest)
[tagzeromqreply]: # (zeromqreply)
[tagrequest]: # (request)
[tagreply]: # (reply)

The Request/Reply message exchange pattern implements a remote procedure call. It involves a client, who creates a `ZeroMQRequest` object, and a server, who creates a `ZeroMQReply` object. The client sends a message to the server, which receives and processes the request and returns a message in response. This sequence of request/reply continues as long as necessary, but in strict order. Messages sent out of order are ignored. 

Minimal client and server communicating via interprocess communication:

    var req = ZeroMQRequest("inproc://endpoint") 
    var rep = ZeroMQReply("inproc://endpoint") 

    req.send("Hello") 
    print rep.receive() 
    rep.send("World!") 
    print req.receive() 

## Publisher-Subscriber
[tagpublisher]: # (publisher)
[tagsubscriber]: # (subscriber)

The Publisher/subscriber pattern is used for data distribution. It involves a publisher, who creates a `ZeroMQPublisher` object and broadcasts messages, and any number of subscribers, who each create a `ZeroMQSubscriber` object and receive messages.

## Push-Pull
[tagpush]: # (push)
[tagpull]: # (pull)

The push/pull message exhange pattern is used for parallel task distribution. It involves two kinds of socket: `ZeroMQPush` used to send messages and `ZeroMQPull` used to receive messages. Both sockets can send and receive from multiple servers and clients. Messages are fair-queued into Pull sockets and load-balanced from Push sockets. This is a pa

## Dealer-Router

Provides a non-blocking request-reply pattern implemented by the `ZeroMQDealer` and `ZeroMQRouter` classes. 
