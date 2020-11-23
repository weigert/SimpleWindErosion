# SimpleWindErosion

C++ implementation of a simple particle-based wind erosion system for terrain generation. This is an adaptation on related particle-based erosion systems, e.g. hydraulic erosion, designed to capture sediment transport, abrasion and cascading.

Rendered using my homebrew [TinyEngine](https://github.com/weigert/TinyEngine).

[Link to a blog post about this](https://weigert.vsos.ethz.ch/2020/11/23/particle-based-wind-erosion/).

## Compiling

Use the makefile to compile the program.

    make all

### Dependencies

    Erosion System:
    - gcc
    - glm
    - libnoise

    Renderer (TinyEngine):
    - SDL2 (Core, Image, TTF, Mixer)
    - OpenGL3
    - GLEW
    - Boost
    - ImGUI (included already as header files)

## Usage

    ./winderosion [SEED]

If no seed is specified, it will take a random one.

### Controls

    - Zoom and Rotate Camera: Scroll
    - Toggle Pause: P (WARNING: PAUSED BY DEFAULT!!)
    - Change Camera Vertical Angle: UP / DOWN
    - Toggle Hydrology Map View: ESC
    - Move the Camera Anchor: WASD / SPACE / C
    
## Screenshots

## Reading

`wind.h` - wind particle movement, sedimentation process
`world.h` - world data storage, generation, rendering
`vegetation.h` - currently inactive

## License
MIT License

See my blog for a more detailed [copyright notice](https://weigert.vsos.ethz.ch/copyright-notice/).
