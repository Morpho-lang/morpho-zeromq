# morpho-zeromq

Morpho extension that provides an interface to the [ZeroMQ](https://zeromq.org) brokerless asynchronous network messaging library and distributed computing framework.

## Installation

The `zeromq` package can be installed with `morphopm`. Type the following into a terminal:

    morphopm install zeromq

## Use

The package can then be loaded into morpho using the `import` keyword.

    import zeromq

You can then create and use ZeroMQ sockets.

    var pull = ZMQPull("inproc://hi")
    var push = ZMQPush("inproc://hi")
    push.send("hello")
    print pull.receive()

For a list of supported functions, start morpho and type

    ? zeromq 

## Examples

Multi-process demos live under `examples/`. Run each peer as a separate process, for example:

    morpho6 examples/reqrep/server.morpho &
    morpho6 examples/reqrep/client.morpho &

See examples/README.md for a complete list of examples.

## Manual installation

To install manually, ensure you have CZMQ (and ZeroMQ) installed using:

    brew install czmq              [macOS]
    apt install libczmq-dev        [ubuntu/WSL]

Then clone this repository onto your computer in any convenient place:

    git clone https://github.com/morpho-lang/morpho-zeromq.git

then add the location of this repository to your .morphopackages file.

    echo PACKAGEPATH >> ~/.morphopackages 
    where PACKAGEPATH is the location of the git repository.

You need to compile the extension, which you can do by cd'ing to the repository's base folder and typing

    cmake -S . -B build
    cmake --build build --config Release
    cmake --install build --config Release

The package can then be loaded into morpho using the `import` keyword.

    import zeromq

## Test suite

Once the extension is installed, run the automated suite from the repository root:

    python3 test/test.py

Use `python3 test/test.py -c` for CI-style reporting (non-zero exit on failure).
