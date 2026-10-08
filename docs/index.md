# OpenW3D API reference

OpenW3D continues the Command & Conquer: Renegade engine released as open source
by Electronic Arts. This reference covers the engine, game code, and tools in
the [OpenW3D repository](https://github.com/w3dhub/OpenW3D).

## Exploring the code

Use the class list, file list, or search box to find a type or function. Source
links open the corresponding code, and inheritance diagrams show how classes
relate to one another.

Useful starting points include:

- \ref Vector3 and \ref Matrix3D for the mathematics library in `Code/WWMath`.
- \ref RefCountClass for reference counting in `Code/wwlib`.
- \ref SmartGameObj for game objects in `Code/Combat`.
- `Code/ww3d2` for rendering, `Code/wwphys` for physics, and `Code/WWAudio` for audio.
- `Code/Tools` for the editors and other development tools.

## Documentation coverage

This reference includes types and functions that do not yet have Doxygen
comments. It is a starting point for navigating the code; detailed explanations
and examples will grow as contributors document the API. Test programs, prebuilt
libraries, and the older core-library copies in `Code/Tools/pluglib` are excluded.

The reference is generated from source without compiling the game. Conditional
compilation and legacy macros can affect which declarations Doxygen displays;
consult the source for platform-specific behavior.

## Contributing

Add documentation comments alongside the declarations they describe, and submit
them through the normal OpenW3D pull request process. See the
[documentation guide](https://github.com/w3dhub/OpenW3D/blob/main/docs/README.md)
for local build instructions and the publishing workflow.

OpenW3D is distributed under GPL v3 with additional terms. See the
[project license](https://github.com/w3dhub/OpenW3D/blob/main/LICENSE.md).
