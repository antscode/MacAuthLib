# MacAuthLib
A C++ library for 68k Macs to communicate with the [MacAuth Web API](https://github.com/antscode/MacAuth).

MacAuthLib is built with the [Retro68 GCC cross-compiler](https://github.com/autc04/Retro68).

See the MacAuthTest application for an example of usage.

## Building & Installing
MacAuthLib requires Retro68 for compilation, and the following libraries:

* [MacWifi](https://github.com/antscode/MacWifi)
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

First build and install the above libraries, then execute these commands from the top level of the MacAuthLib directory:

    cd ..
    mkdir MacAuthLib-build
    cd MacAuthLib-build
    cmake ../MacAuthLib -DCMAKE_TOOLCHAIN_FILE=<<YOUR_PATH_TO_Retro68-build>>/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
    make install

This will build and install the library and headers into the m68k-apple-macos toolchain.

The MacAuthTest application will be in the MacAuthLib-build directory.

## Credits
MacAuthLib uses code from the following open source projects:

* [autc04/Retro68](https://github.com/autc04/Retro68)
* [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
