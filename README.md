# morpho-zeromq

Morpho extension that provides an interface to the [ZeroMQ](https://zeromq.org) brokerless asynchronous network messaging library and distributed computing framework.

## Installation

To install this, ensure you have zeromq installed using:

    brew install czmq    [macOS]
    apt get libczmq-dev  [ubuntu/WSL]

Then clone this repository onto your computer in any convenient place:

    git clone https://github.com/morpho-lang/morpho-zeromq.git

then add the location of this repository to your .morphopackages file.

    echo PACKAGEPATH >> ~/.morphopackages 
    where PACKAGEPATH is the location of the git repository.

You need to compile the extension, which you can do by cd'ing to the repository's base folder and typing

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release .. 
    make install

The package can then be loaded into morpho using the `import` keyword.

    import zeromq
