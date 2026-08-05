# Examples

Multi-process demos for `morpho-zeromq`. Run each peer in its own terminal from the package root (after `import zeromq` works).

## Patterns

### `reqrep/`

Classic request/reply. Start the server, then the client:

    morpho6 examples/reqrep/server.morpho
    morpho6 examples/reqrep/client.morpho

### `pubsub/`

Publish/subscribe weather-style stream. Start the publisher, then the subscriber:

    morpho6 examples/pubsub/publisher.morpho
    morpho6 examples/pubsub/subscriber.morpho

### `lazypirate/`

Reliable request/reply (Lazy Pirate): client timeouts, reconnects, and retries while the server randomly overloads, corrupts replies, or crashes.

    morpho6 examples/lazypirate/lpserver.morpho
    morpho6 examples/lazypirate/lpclient.morpho

## ZeroMQ Guide (Chapter 1)

Ports of the [ZeroMQ Guide](https://zguide.zeromq.org/) Chapter 1 samples, under `zguide/chapter1/`.

### `HelloWorld/`

Minimal REQ/REP hello world.

    morpho6 examples/zguide/chapter1/HelloWorld/helloworldserver.morpho
    morpho6 examples/zguide/chapter1/HelloWorld/helloworldclient.morpho

### `WeatherUpdate/`

PUB/SUB weather updates filtered by zip code.

    morpho6 examples/zguide/chapter1/WeatherUpdate/weatherupdateserver.morpho
    morpho6 examples/zguide/chapter1/WeatherUpdate/weatherupdateclient.morpho

### `TaskVentilator/`

PUSH/PULL pipeline: a ventilator fans work out to workers, and a sink collects results. Start the sink and one or more workers, then the ventilator (or use `testtaskventilator.morpho` as a convenience launcher from that directory).

    morpho6 examples/zguide/chapter1/TaskVentilator/tasksink.morpho
    morpho6 examples/zguide/chapter1/TaskVentilator/taskworker.morpho
    morpho6 examples/zguide/chapter1/TaskVentilator/taskvent.morpho
