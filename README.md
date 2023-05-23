# rtac_asio

This is a collection of utilities to exchange data other TCP/UDP sockets and
serial ports.

This is mainly a wrapper around boost::asio for ease of use and integration in
other projects.

It was solely tested on Ubuntu 18-20, and a few other Debian based distributions
(such on a Raspberry PI 3 with Rasbian) but should be reasonably cross-platform
with little tweaks. Feedback of use on other platforms would be appreciated.

# Installation

## System dependencies (Ubuntu)

This package solely depends on boost. And you will need a compiler and CMake.

```
sudo apt-get install -y libspdlog-dev libboost-thread-dev libboost-system-dev build-essential cmake
```

## Install rtac_asio

This is a standard modern CMake package.

```
git clone https://github.com/pnarvor/rtac_asio.git

cd rtac_asio

mkdir build && cd build

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<your_install_location> ..

make install
```

Make sure <your_install_location> is listed under the CMAKE_PREFIX_PATH
environment variable.

```
printenv | grep CMAKE_PREFIX_PATH
```

If you don't see anything, or <your_install_location> is not listed in the
output, add the following line at the end of your $HOME/.bashrc file (or whatever
file is used to setup your terminal environment).

```
export CMAKE_PREFIX_PATH=<your_install_location>:$CMAKE_PREFIX_PATH
```

# Integration in your CMake project

Add these lines to the CMakeLists.txt of your project :

```
find_package(rtac_asio REQUIRED)


target_link_libraries(<your_target> PUBLIC rtac_asio)
```


# Usage

You can find some examples in the tests/src directory.

More examples and a wiki are coming soon.

In the meantime, take a look at the include/rtac_asio/Stream.h file. The Stream
class is the only class you need to instanciate in your project. It can
represent a Serial port, or a UDP/TCP client.








