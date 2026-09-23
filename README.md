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

Clone this repository onto your computer in any convenient place:

    git clone https://github.com/morpho-lang/morpho-zeromq.git

then add the location of this repository to your .morphopackages file.

    echo PACKAGEPATH >> ~/.morphopackages 
    where PACKAGEPATH is the location of the git repository.

### macOS / Linux

Install CZMQ (and ZeroMQ) using:

    brew install czmq              [macOS]
    apt install libczmq-dev        [ubuntu/WSL]

Then compile the extension from the repository's base folder:

    cmake -S . -B build
    cmake --build build --config Release
    cmake --install build --config Release

### Windows

Requires Visual Studio 2022 with the ClangCL toolset and a bootstrapped [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started) (for example at `C:\vcpkg`). Create a local `CMakeUserPresets.json` that points the `vcpkg` preset at your install, e.g.:

```json
{
  "version": 2,
  "configurePresets": [
    {
      "name": "default",
      "inherits": "vcpkg",
      "environment": {
        "VCPKG_ROOT": "C:/vcpkg"
      }
    }
  ]
}
```

Manifest mode then downloads and builds CZMQ and ZeroMQ on first configure (this may take several minutes) and links them statically into the plugin, so the resulting DLL has no extra ZeroMQ dependencies.

    cmake --preset default
    cmake --build build --config Release
    cmake --install build --config Release

The package can then be loaded into morpho using the `import` keyword.

    import zeromq

## Test suite

Once the extension is installed, run the automated suite from the repository root:

    python3 test/test.py

Use `python3 test/test.py -c` for CI-style reporting (non-zero exit on failure).
