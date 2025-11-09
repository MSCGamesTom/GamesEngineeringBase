# GamesEngineeringBase Library Documentation

The **GamesEngineeringBase** library is a collection of utility classes designed to facilitate game development in a single header file without the need for any external libraries. This is designed for the MSc Games Engineering course at the University of Warwick. It provides functionality for window management, rendering, audio playback, input handling, and timing. The library is organized within the `GamesEngineeringBase` namespace and is licensed under the MIT License.

## Table of Contents

- [Introduction](#introduction)
- [Namespace Overview](#namespace-overview)
- [Classes](#classes)
  - [Window](#window)
  - [Sound](#sound)
  - [SoundManager](#soundmanager)
  - [Timer](#timer)
  - [Image](#image)
  - [XBoxController](#xboxcontroller)
  - [XBoxControllers](#xboxcontrollers)
- [Usage Examples](#usage-examples)
- [License](#license)

## Introduction

The **GamesEngineeringBase** library simplifies the setup and management of essential game components. It abstracts the complexities of DirectX initialization, sound loading and playback, image handling, and controller input, allowing developers to focus on game logic and design.

## Namespace Overview

All classes are encapsulated within the `GamesEngineeringBase` namespace to prevent naming conflicts and to logically group the various functionalities.

```cpp
namespace GamesEngineeringBase
{
    // Classes and definitions
}
```

## Classes

### Window

The `Window` class manages the creation and handling of a Windows application window with DirectX 11 rendering capabilities.

#### Key Features

- Window creation with customizable size, title, and fullscreen mode.
- DirectX 11 device and swap chain initialization.
- Input handling for keyboard and mouse events.
- Rendering support with a back buffer for pixel manipulation.

#### Public Methods

- `void create(unsigned int window_width, unsigned int window_height, const std::string window_name, bool window_fullscreen = false, int window_x = 0, int window_y = 0);`
  - Initializes and creates the window with specified parameters.
- `void checkInput();`
  - Processes pending input messages.
- `unsigned char* backBuffer() const;`
  - Returns a pointer to the back buffer for pixel access.
- `void draw(int x, int y, unsigned char r, unsigned char g, unsigned char b);`
  - Draws a pixel at (x, y) with the given RGB color.
- `void draw(int pixelIndex, unsigned char r, unsigned char g, unsigned char b);`
  - Draws a pixel at pixelIndex offset with the given RGB color. Note pixelIndex is computed as (y * width) + x.
- `void draw(int x, int y, unsigned char* pixel);`
  - Draws a pixel at (x, y) using the color from the provided pixel array.
- `void clear();`
  - Clears the back buffer.
- `void present();`
  - Presents the back buffer to the screen.
- `unsigned int getWidth() const;`
  - Returns the window's width.
- `unsigned int getHeight() const;`
  - Returns the window's height.
- `bool keyPressed(int key) const;`
  - Checks if a specific key is pressed. Letter and number keys can be accessed via passing in the appropriate char, i.e. `keyPressed('A')` detects if the A key is pressed. Special keys can be accessed via the [Windows Virtual Key codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes).
- `bool mouseButtonPressed(MouseButton button) const;`
  - Checks if a specific mouse button is currently pressed. Accepts one of `MouseButton` enum as an input. That is `MouseLeft` for the left button, `MouseMiddle` for the middle button, and `MouseRight` for the right button.
- `MouseButtonState mouseButtonState(MouseButton button) const;`
  - Returns the current state of a mouse button (`MouseUp`, `MouseDown`, or `MousePressed`).
- `int getMouseX() const;`
  - Returns the current X-coordinate of the mouse cursor within the window.
- `int getMouseY() const;`
  - Returns the current Y-coordinate of the mouse cursor within the window.
- `int getMouseWheel() const;`
  - Returns the mouse wheelâ€™s current scroll value.
- `void resetMouseWheelPosition();`
  - Resets the mouse wheel scroll value to 0.
- `int getMouseInWindowX() const;`
  - Gets the mouse X-coordinate relative to the window.
- `int getMouseInWindowY() const;`
  - Gets the mouse Y-coordinate relative to the window.
- `void clipMouseToWindow() const;`
  - Restricts the mouse cursor to the window's client area.
- `unsigned char* getBackBuffer() const;`
  - Returns a pointer to the raw back buffer data for low-level access or screenshots.

### Sound

The `Sound` class handles the loading and playback of WAV audio files using XAudio2.

#### Key Features

- Loading WAV files into audio buffers.
- Playing sounds with support for multiple instances.
- Infinite looping for background music.

#### Public Methods

- `bool loadWAV(IXAudio2* xaudio, std::string filename);`
  - Loads a WAV file into the sound buffer.
- `void play();`
  - Plays the sound once.
- `void playMusic();`
  - Plays the sound in an infinite loop.

### SoundManager

The `SoundManager` class manages multiple `Sound` instances and provides an interface for sound playback.

#### Key Features

- Centralized management of sound resources.
- Loading and playing of sound effects and music.
- Resource cleanup and management.

#### Public Methods

- `SoundManager();`
  - Constructor that initializes XAudio2.
- `void load(std::string filename);`
  - Loads a sound effect.
- `void play(std::string filename);`
  - Plays a loaded sound effect.
- `void loadMusic(std::string filename);`
  - Loads a music track.
- `void playMusic();`
  - Plays the loaded music track.

### Timer

The `Timer` class provides high-resolution timing functionality using performance counters.

#### Key Features

- High-precision time measurement.
- Delta time calculation for frame-independent movement.

#### Public Methods

- `Timer();`
  - Constructor that initializes the frequency.
- `void reset();`
  - Resets the timer.
- `float dt();`
  - Returns the elapsed time since the last call to dt() or reset() in seconds.

### Image

The `Image` class handles image loading and pixel data manipulation using Windows Imaging Component (WIC).

#### Key Features

- Loading images in the following formats: PNG, JPEG, BMP, TIFF, DDS.
- Accessing pixel data with support for different channels.
- Alpha channel handling for transparency.

#### Public Methods

- `bool load(std::string filename);`
  - Loads an image file.
- `unsigned char* at(unsigned int x, unsigned int y) const;`
  - Returns a pointer to the pixel data at the specified coordinates. These coordinates are clamped to be within image bounds.
- `unsigned char alphaAt(unsigned int x, unsigned int y) const;`
  - Returns the alpha value of the pixel at the specified coordinates. These coordinates are clamped to be within image bounds.
- `unsigned char at(unsigned int x, unsigned int y, unsigned int index) const;`
  - Returns the color specified by index at (x, y) coordinates. Index should be between 0 and 2, where 0 == Red, 1 == Green, 2 == Blue.
- `unsigned char* atUnchecked(unsigned int x, unsigned int y) const;`
  - Returns a pointer to the pixel data at the specified coordinates. This is faster than `at()` as these coordinates are *not* clamped to be within image bounds.
- `unsigned char alphaAtUnchecked(unsigned int x, unsigned int y) const;`
  - Returns the alpha value of the pixel at the specified coordinates. This is faster than `alphaAt()` as these coordinates are *not* clamped to be within image bounds.
- `bool hasAlpha() const;`
  - Checks if the image contains an alpha channel.
- `void free();`
  - Frees the allocated image data.

### XBoxController

The `XBoxController` class represents a single Xbox controller and provides methods to access its state.

#### Key Features

- Reading controller input states.
- Normalizing thumbstick values with deadzone handling.
- Trigger and button state access.
- Vibration support.

#### Public Methods

- `XBoxController();`
  - Constructor that initializes the controller ID to -1 (inactive).
- `void activate(int _ID);`
  - Activates the controller with the specified ID.
- `void deactivate();`
  - Deactivates the controller.
- `void update();`
  - Updates the controller's state.
- Button state methods like `bool APressed();`, `bool BPressed();`, etc.
- `void vibrate(float l, float r);`
  - Sets the vibration intensity for the left and right motors.
- `int getID();`
  - Returns the controller ID.

### XBoxControllers

The `XBoxControllers` class manages multiple `XBoxController` instances and detects connected controllers.

#### Key Features

- Detection of connected Xbox controllers.
- Access to individual controllers.
- Checking for any connected controllers.

#### Public Methods

- `XBoxControllers();`
  - Constructor that probes for connected controllers.
- `XBoxController getPlayerController(int index);`
  - Returns the controller at the specified index.
- `XBoxController getFirstPlayerController();`
  - Returns the first active controller.
- `bool hasController();`
  - Checks if any controller is connected.
- `void probeControllers();`
  - Updates the list of connected controllers.

## Usage Examples

### Creating a Window and Rendering Pixels

```cpp
#include "GamesEngineeringBase.h"

using namespace GamesEngineeringBase;

int main()
{
    Window window;
    window.create(800, 600, "My Game Window");

    // Main loop
    while (true)
    {
        window.checkInput();

        // Clear the back buffer
        window.clear();

        // Draw some pixels
        for (int x = 0; x < window.getWidth(); x++)
        {
            for (int y = 0; y < window.getHeight(); y++)
            {
                window.draw(x, y, 255, 0, 0); // Red color
            }
        }

        // Present the back buffer
        window.present();
    }

    return 0;
}
```

### Playing a Sound Effect

```cpp
#include "GamesEngineeringBase.h"

using namespace GamesEngineeringBase;

int main()
{
    SoundManager soundManager;
    soundManager.load("explosion.wav");

    // Play the sound effect
    soundManager.play("explosion.wav");

    return 0;
}
```

### Loading and Displaying an Image

```cpp
#include "GamesEngineeringBase.h"

using namespace GamesEngineeringBase;

int main()
{
    Window window;
    window.create(800, 600, "Image Display");

    Image image;
    image.load("picture.png");

    while (true)
    {
        window.checkInput();

        // Clear the back buffer
        window.clear();

        // Display the image
        for (unsigned int x = 0; x < image.width; x++)
        {
            for (unsigned int y = 0; y < image.height; y++)
            {
                unsigned char* pixel = image.at(x, y);
                window.draw(x, y, pixel[0], pixel[1], pixel[2]);
            }
        }

        // Present
        window.present();
    }

    return 0;
}
```

## License

This library is licensed under the MIT License.
