# BatEngine

## About
A hobby game engine project. Still in early stages and not very usable. [![Build status](https://ci.appveyor.com/api/projects/status/q13c8448sfldefpy?svg=true)](https://ci.appveyor.com/project/SlidyBat/batengine)

## Platforms
 * Windows

## Structure
At the moment there is no clear distinction between engine/client code. Everything is just clumped into the `Engine` project.
Typically `Application.cpp` will contain test code that tests the features I'm currently working on. I plan to make a separate testbed/example application that showcases all the features of the engine some time in the future. All engine code is in the `Bat` namespace.

### Directories
 - **ThidParty** - Contains all the dependencies of the engine:
    - [Assimp](https://github.com/assimp/assimp)
        Mesh/scene loading.
    - [DirectXTK](https://github.com/Microsoft/DirectXTK)
        Texture loading & font drawing.
    - [ENet](http://enet.bespin.org/)
        Used for networking.
    - [ImGui](https://github.com/ocornut/imgui/)
        Quick debug menus. Should use the engine UI system for actual menus.
    - [PhysX](https://github.com/NVIDIAGameWorks/PhysX)
        For physics system.
    - [Ultralight](https://ultralig.ht)
        HTML rendering library. Used for UI system.
    - [irrKlang](https://www.ambiera.com/irrklang/)
        Used for playing/recording audio.
    - [spdlog](https://github.com/gabime/spdlog)
        Logging library used to print logs to console in debug builds.
 - **Assets** - This contains all the media used in the engine. Most of these are used for testing only.
 - **Util** - Contains general utility files that don't depend on any other parts of the engine. Used frequently by all parts of the engine.
 - **Core** - Contains core engine systems that tie together all other engine systems.
 - **Platform** - Platform specific code. ATM only contains Windows code, other platforms are unsupported.
 - **Events** - Contains event dispatcher/listener classes implementation as well as all the event types used by the engine systems.
 - **Graphics** - All graphics/rendering related code. Relies on D3D11. See `Graphics` class in `Graphics.h` for an overview of whats available.
 - **Networking** - All networking related code. Relies on ENet. See `Networking` class in `Networking.h` for an overview of whats available.
 - **Audio** - All audio related code. Relies on irrKlang. See `Audio` class in `Audio.h` for an overview of whats available.
 - **Physics** - All physics related code. Relies on PhysX. See `Physics` class in `Physics.h` for an overview of whats available.

## Building
Windows is the only supported platform. Use Visual Studio 2017 (or higher) to open Engine.sln and build the solution. Output will be in the `Build` directory.

## Packaging
Requires all .exe/.dll files in the Build directory as well as the assets in /Assets and shaders in /Graphics/Shaders
