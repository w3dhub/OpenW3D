
# OpenW3D

This project is a continuation of the "Command & Conquer: Renegade" game engine that was released as open source software by EA [here](https://github.com/electronicarts/CnC_Renegade).
It is being developed to be more portable to other platforms, to fix bugs and to develop a platform to build other Renegade like games on top of.

## Dependencies

OpenW3D makes use of several open source libraries to provide cross platform support.
In order to build the project they will need to be made available to the build system either via a package manager or manually installing and updating your path environment variables.

For convenience we provide support for using vcpkg to install the required dependencies:

[SDL3](https://wiki.libsdl.org/SDL3/FrontPage)

[OpenAL-soft](https://openal-soft.org/)

[FFMpeg](https://www.ffmpeg.org/)

[FreeType](https://freetype.org/)

[ICU4C](https://icu.unicode.org/)

[Qt](https://www.qt.io/)

In addition to these, we also use these less commonly packaged libraries that are pulled in by the build system. You do not need to provide these manually unless you are building off line:

[Crunch](https://github.com/binomialllc/crunch)

[GameSpySDK](https://github.com/TheAssemblyArmada/GamespySDK)


## Compiling the Project

For most platforms the following command should build the project with the default configuration when run from the root folders of a cloned copy of this repository:

`cmake -B build . && cmake --build build`

The build artifacts will end up in the `build` subdirectory which will be created.

It is also possible to build from various IDE by using the presets provided in `CMakePresets.json`.

You can also create your own prests by creating and populating `CMakeUserPresets.json` in the root folder of the cloned repository. This is useful for development or testing purposes to create additional build options.
See [cmake-presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) for more information.

## Running the Game

To use the compiled binaries, you must provide game data. At the time of writing only the original C&C: Renegade game is tested and supported.
The C&C Ultimate Collection is available for purchase on [EA App](https://www.ea.com/en-gb/games/command-and-conquer/command-and-conquer-the-ultimate-collection/buy/pc) or [Steam](https://store.steampowered.com/bundle/39394/Command__Conquer_The_Ultimate_Collection/).
The compiled binaries will also work with a fully patched installation from the original CDs.

## Known Issues

The “Debug” configuration of the “Commando” project (the Renegade main project) will sometimes fail to link the final executable.
This is due to Windows Defender incorrectly detecting RenegadeD.exe containing a virus (possibly due to the embedded browser code).
Excluding the output `/Run/` folder found in the root of this repository in Windows Defender should resolve this for you.

## Contributing

We welcome contributions to the project! If you’re interested in contributing, you need to have knowledge of C++.
Join the developer chat on [Discord](https://discord.com/invite/jMmmRa2) for more information on how to get started.
Please make sure to read our [Contributing Guidelines](CONTRIBUTING.md) before submitting a pull request.

## License

EA has not endorsed and does not support this project. All trademarks are the property of their respective owners.

This repository and its contents are licensed under the GPL v3 license, with additional terms applied. Please see [LICENSE.md](LICENSE.md) for details.
