[comment]: # (ZeroMQ help)
[version]: # (0.7)

# ZeroMQ
[tagZeroMQ]: # (ZeroMQ)
[tagZMQ]: # (ZMQ)
[tag0MQ]: # (0MQ)

The `ZeroMQ` extension provides support for distributed computing through the `ZeroMQ` library. The API provided is designed to be high-level, at least relative to other language bindings, and follows the `czmq` patterns rather than the low-level `zmq` patterns. To use the `ZeroMQ` extension import it,

    import zeromq

You then create appropriate `ZMQSocket` objects and bind or connect to them by calling the appropriate constructor. `ZeroMQ` supports multiple types of transport, including interprocess, tcp, etc.; see the examples or `czmq`/`ZeroMQ` documentation for further details. See `ZMQSocket` for general information about sockets.

Once appropriate sockets have been created, use the `send` and `receive` methods to send information.

ZeroMQ failures raise a catchable `ZMQErr`; receive timeouts return `nil`.

    try {
      sock.bind("bad://endpoint")
    } catch {
      "ZMQErr": print "bind failed"
    }

[showsubtopics]: # (subtopics)

## ZMQSocket
[tagsocket]: # (socket)
[tagzmqsocket]: # (ZMQSocket)

The `ZMQSocket` class provides access to `ZeroMQ` sockets, which abstract many kinds of inter-process and inter-machine transports into a single convenient API. `ZeroMQ` provides several *patterns*, reusable kinds of socket, that each have slightly different behavior and are designed to solve recurring problems in network architecture.

To create a socket, use the appropriate constructor, e.g.

    var requester = ZMQRequest("tcp://localhost:5555")

which connects to a TCP connection on port 5555. Sockets either *bind* (create) an endpoint, or *connect* (join) a specified endpoint. Generally, "stable" parts of your architecture, those that are persistent, should bind to an endpoint and "dynamic" parts should connect.

Some examples of different transports.

    var rep = ZMQReply("tcp://*:5555") // Bind to a specified port on any available interface.
    var rep = ZMQReply("tcp://127.0.0.1:5555") // Bind to a specified port on local loopback interface.

    var req = ZMQRequest("tcp://localhost:5555") // Connect to a specified local port
    var req = ZMQRequest("ipc:///tmp/feeds/0") // Connect to inter-process communication on UNIX only
    var req = ZMQRequest("inproc://endpoint-string") // Connect to an in-process communication socket.

Each socket type has a default action, either bind or connect. To override, simply create the socket without an endpoint specified and use the `bind` or `connect` methods:

    var req = ZMQRequest()
    req.bind("tcp://*:5555")

For further information, consult the `czmq` documentation.

[showsubtopics]: # (subtopics)

### Bind
[tagbind]: # (bind)

Binds a socket to an endpoint, creating it if necessary:

    var rep = ZMQReply()
    rep.bind("tcp://*:5555")

### Unbind
[tagunbind]: # (unbind)

Unbinds a socket from an endpoint. With no argument, unbinds the current endpoint returned by `endpoint`. Calling `unbind()` when the socket is not bound is a no-op:

    rep.unbind("tcp://*:5555")
    sock.unbind()
    sock.unbind() // no-op

### Connect
[tagconnect]: # (connect)

Connects a socket to an existing endpoint:

    var req = ZMQRequest()
    req.connect("tcp://localhost:5555")

### Disconnect
[tagdisconnect]: # (disconnect)

Disconnects a socket from an endpoint:

    req.disconnect("tcp://localhost:5555")

### Endpoint
[tagendpoint]: # (endpoint)

Returns the endpoint a socket is currently bound to, or `nil` if none (including after `unbind`):

    var pull = ZMQPull("tcp://127.0.0.1:!")
    print pull.endpoint() // e.g. tcp://127.0.0.1:5555
    pull.unbind()
    print pull.endpoint() // nil

### Send
[tagsend]: # (send)

Sends a message. Pass a `String` for a single-frame message, or a `Tuple` / `List` of strings for a multipart message:

    sock.send("hello")
    sock.send(("id", "", "hello"))

### Receive
[tagreceive]: # (receive)

Receives a message. Returns a `String` for a single-frame message, a `Tuple` of strings for multipart messages, or `nil` on timeout:

    var msg = sock.receive()

### Subscribe
[tagsubscribe]: # (subscribe)

Adds a subscription filter on a subscriber socket:

    var sub = ZMQSubscriber("tcp://localhost:5556")
    sub.subscribe("10001")

An initial filter can also be passed to the `ZMQSubscriber` constructor as a second argument.

### Unsubscribe
[tagunsubscribe]: # (unsubscribe)

Removes a subscription filter:

    sub.unsubscribe("10001")

### Timeout
[tagtimeout]: # (timeout)

Sets or gets the receive timeout in milliseconds. With no argument, returns the current timeout (`-1` means wait forever). A timed-out `receive` returns `nil`:

    sock.timeout(2500)
    print sock.timeout()
    var msg = sock.receive() // nil if nothing arrives within 2500 ms

### Linger
[taglinger]: # (linger)

Sets or gets the linger period in milliseconds (how long a socket waits on close to flush pending messages):

    sock.linger(0)
    print sock.linger()

### Identity
[tagidentity]: # (identity)

Sets or gets the socket identity string (supported on socket types such as `ZMQDealer` and `ZMQRequest`):

    var dealer = ZMQDealer()
    dealer.identity("worker")
    print dealer.identity()

### Unbounded
[tagunbounded]: # (unbounded)

Sets infinite high-water marks on the socket:

    sock.unbounded()

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

## XPublisher-XSubscriber
[tagxpublisher]: # (xpublisher)
[tagxsubscriber]: # (xsubscriber)
[tagZMQxpublisher]: # (ZMQXPublisher)
[tagZMQxsubscriber]: # (ZMQXSubscriber)

`ZMQXPublisher` and `ZMQXSubscriber` are the raw forms of publisher and subscriber, used mainly when building proxies or brokers. Ordinary pub/sub hides subscription traffic; these sockets expose it as messages so an intermediary can forward subscriptions upstream.

- `ZMQXPublisher` behaves like `ZMQPublisher`, but can also *receive* subscription messages from peers: a leading byte `1` (subscribe) or `0` (unsubscribe), followed by the topic.
- `ZMQXSubscriber` behaves like `ZMQSubscriber`, but subscriptions are *sent* as those same framed messages rather than only via `subscribe` / `unsubscribe`.

Typical proxy layout:

    // Publishers connect here; subscribers connect to the XPUB side
    proxy.setfrontend(ZMQXSubscriber, "tcp://*:5559")
    proxy.setbackend(ZMQXPublisher, "tcp://*:5560")

By default, `ZMQXPublisher` *binds* to an endpoint, and `ZMQXSubscriber` *connects*.

## Push-Pull
[tagpush]: # (push)
[tagpull]: # (pull)

The push/pull message exchange pattern is used for parallel task distribution. It involves two kinds of socket: `ZMQPush` used to send messages and `ZMQPull` used to receive messages. Both sockets can send and receive from multiple servers and clients. Messages are fair-queued into Pull sockets and load-balanced from Push sockets.

By default, `ZMQPush` *connects* to an endpoint, and `ZMQPull` *binds*.

## Dealer-Router
[tagdealer]: # (dealer)
[tagrouter]: # (router)

Provides a non-blocking request-reply pattern implemented by the `ZMQDealer` and `ZMQRouter` classes. Messages between them are multipart: a `ZMQRouter` receives a `Tuple` whose first element is the peer identity, followed by the message frames.

    var router = ZMQRouter("inproc://endpoint")
    var dealer = ZMQDealer()
    dealer.identity("client")
    dealer.connect("inproc://endpoint")

    dealer.send("Hello")
    var req = router.receive()   // ("client", "Hello")
    router.send((req[0], "World"))
    print dealer.receive()       // World

By default, `ZMQRouter` *binds* to an endpoint, and `ZMQDealer` *connects*.

## Pair
[tagpair]: # (pair)
[tagZMQpair]: # (ZMQpair)

`ZMQPair` provides exclusive one-to-one communication between exactly two sockets. Unlike the other patterns, each pair peer is the same socket type: both ends create a `ZMQPair`.

    var a = ZMQPair()
    a.bind("inproc://pair")
    var b = ZMQPair()
    b.connect("inproc://pair")

    a.send("ping")
    print b.receive() // ping

One side should *bind* and the other *connect*. Pair sockets are useful for simple inter-thread or inter-process links where you do not need fan-out or fair-queuing.

## ZMQPoller
[tagZMQPoller]: # (ZMQPoller)
[tagpoller]: # (poller)

`ZMQPoller` waits until one or more sockets are ready to receive. Pass the sockets to poll as constructor arguments:

    var client = ZMQRequest("tcp://127.0.0.1:5555")
    var poller = ZMQPoller(client)

For a single socket, prefer `sock.timeout(ms)` and checking whether `receive()` returned `nil`.

[showsubtopics]: # (subtopics)

### Wait
[tagwait]: # (wait)

Waits until a socket is ready to receive. Returns the ready socket, or `nil` on timeout. With no argument, blocks indefinitely:

    var ready = poller.wait(2500)
    if (ready==client) print client.receive()

    poller.wait() // block until ready

### Add
[tagadd]: # (add)

Adds a socket to the poller:

    poller.add(another)

### Remove
[tagremove]: # (remove)

Removes a socket from the poller:

    poller.remove(client)

## ZMQProxy
[tagZMQProxy]: # (ZMQProxy)
[tagproxy]: # (proxy)

`ZMQProxy` runs a built-in message proxy between a frontend and a backend socket type:

    var proxy = ZMQProxy()
    proxy.setfrontend(ZMQPull, "inproc://frontend")
    proxy.setbackend(ZMQPush, "inproc://backend")

    // Application sockets connect to the proxy endpoints
    var faucet = ZMQPush(">inproc://frontend")
    var sink = ZMQPull(">inproc://backend")

    faucet.send("Hello World")
    print sink.receive()

[showsubtopics]: # (subtopics)

### Setfrontend
[tagsetfrontend]: # (setfrontend)

Configures the proxy frontend with a socket constructor and endpoint:

    proxy.setfrontend(ZMQPull, "inproc://frontend")

### Frontend
[tagfrontend]: # (frontend)

Returns the configured frontend endpoint string:

    print proxy.frontend()

### Setbackend
[tagsetbackend]: # (setbackend)

Configures the proxy backend with a socket constructor and endpoint:

    proxy.setbackend(ZMQPush, "inproc://backend")

### Backend
[tagbackend]: # (backend)

Returns the configured backend endpoint string:

    print proxy.backend()

### Pause
[tagpause]: # (pause)

Temporarily stops the proxy:

    proxy.pause()

### Resume
[tagresume]: # (resume)

Restarts a paused proxy:

    proxy.resume()
