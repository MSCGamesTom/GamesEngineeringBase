/*
MIT License

Copyright (c) 2024 MSc Games Engineering Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "GamesEngineeringBase.h" // Include the GamesEngineeringBase header

int main(int argc, char* argv[])
{
    // Create a canvas window with dimensions 1024x768 and title "Example"
    GamesEngineeringBase::Window canvas;
    canvas.create(1024, 768, "Example");
    bool running = true; // Variable to control the main loop's running state.

    // Load an image and place it at coordinates (300, 300)
    GamesEngineeringBase::Image image;
    image.load("Resources/A.png");
    unsigned int planeX = 300;  // Initial x-coordinate for the plane image.
    unsigned int planeY = 300;  // Initial y-coordinate for the plane image.

    // Load and play background music and an explosion sound
    GamesEngineeringBase::SoundManager sounds;
    sounds.loadMusic("Resources/music.wav");
    sounds.playMusic();
    sounds.load("Resources/explosion.wav"); // Preload explosion sound.

    // Timer object to manage time-based events, such as movement speed
    GamesEngineeringBase::Timer timer;
    timer.reset(); // Reset the timer at the start of the game.

    // Define a move speed for the plane, in this case it is pixels moved per second
    float MOVE_SPEED = 1000.0f;

    // Main game loop
    while (running)
    {
        // Check for input (key presses or window events)
        canvas.checkInput();

        // Clear the window for the next frame rendering
        canvas.clear();

        // If the Escape key is pressed, exit the loop and close the window
        if (canvas.keyPressed(VK_ESCAPE))
        {
            break; // Exits the game loop
        }

        // Calculate how far the plane should move based on elapsed time (ensuring a minimum movement of 1 pixel)
        unsigned int move_amount = static_cast<unsigned int>(max(MOVE_SPEED * timer.dt(), 1.0f));

        // Control the plane's movement using W, S, A, D keys
        if (canvas.keyPressed('W')) // Move plane up
        {
            planeY = planeY - move_amount;// Speed is adjusted based on time delta.
        }
        if (canvas.keyPressed('S')) // Move plane down
        {
            planeY = planeY + move_amount;
        }
        if (canvas.keyPressed('A')) // Move plane left
        {
            planeX = planeX - move_amount;
        }
        if (canvas.keyPressed('D')) // Move plane right
        {
            planeX = planeX + move_amount;
        }

        // Check if the plane goes out of bounds (off the screen)
        if (planeX > (canvas.getWidth() - image.width) || planeX < 0 || planeY > (canvas.getHeight() - image.height) || planeY < 0)
        {
            // Play the explosion sound when plane hits the edge
            sounds.play("Resources/explosion.wav");

            // Reset plane's position to the center of the canvas
            planeX = (canvas.getWidth() / 2) - (image.width / 2);
            planeY = (canvas.getHeight() / 2) - (image.height / 2);
        }

        // Clear the entire canvas with a blue background (RGB: 0, 0, 255)
        for (unsigned int i = 0; i < (canvas.getWidth() * canvas.getHeight()); i++)
        {
            canvas.draw(i, 0, 0, 255); // Drawing the blue background pixel by pixel.
        }

        // Draw the plane image at the calculated coordinates
        for (unsigned int i = 0; i < image.height; i++) // Loop through the image height
        {
            for (unsigned int n = 0; n < image.width; n++) // Loop through the image width
            {
                // Ensure drawing stays within canvas bounds
                if ((planeX + n) >= 0 && (planeX + n) < canvas.getWidth() && (planeY + i) >= 0 && (planeY + i) < canvas.getHeight())
                {
                    // Only draw the pixel if the alpha transparency of the pixel is above some value (in this case 210)
                    if (image.alphaAt(n, i) > 210)
                    {
                        canvas.draw(planeX + n, planeY + i, image.at(n, i)); // Draw the plane's pixel onto the canvas
                    }
                }
            }
        }

        // Reset the timer for the next frame, so movement is smooth and time-based.
        timer.reset();

        // Display the frame on the screen. This must be called once the frame is finished in order to display the frame.
        canvas.present();
    }
}
