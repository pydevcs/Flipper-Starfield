# Apple Silicon Simulator and FlipperZero Firmware Build

This repository contains instructions for building and running the Apple Silicon Simulator (Starfield) and the Flipper Firmware.

## Apple Silicon Simulator (Starfield)

The Apple Silicon Simulator is a simple starfield simulation that you can build and run on macOS with an Apple silicon chip. Follow the steps below to compile and run the simulator.

### Prerequisites

- macOS with an Apple Silicon chip
- GCC (installed via Homebrew)
- SDL2 (installed via Homebrew)

### Install Dependencies

First, ensure that you have the required dependencies:

```bash
brew install gcc sdl2

cd darwin

gcc starfield.c -o starfield -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lm

./starfield
```

Press ENTER to invert screen colors.

Left & Right arrow keys to move left or right.

Up & Down arrow keys to increase or decrease warp speed.


## FlipperZero Firmware Build (Momentum):

```bash
cd flipper

ufbt update --index-url https://up.momentum-fw.dev/firmware/directory.json

ufbt launch APPSRC=starfield
```

Press SELECT button to toggle the screen color inversion.

Left & Right button to move left or right.

Up & Down button to increase or decrease warp speed.
