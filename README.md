# Asteroid Avoider
This is the source code of the project created for the Microcontroller Programming course. It is an arcade-style survival game where you pilot a spaceship, dodge incoming asteroids, and try to survive as long as possible to set a new high score.

## Key Features

* **Custom Differential Rendering:** To maintain a smooth, flicker-free framerate on the microcontroller, the game erases and redraws the specific pixels that moved from one frame to another, completely avoiding full-screen clears that would slow down the game.
* **Advanced 2D Physics:** The game features a custom physics engine with elastic collisions between asteroids. To prevent fast-moving objects from getting stuck inside each other, the game calculates the exact fraction of a second they touched and "rewinds" them to that perfect point of impact so they bounce cleanly. It also includes knockback mechanics for the player when hit.

## Hardware Components

* ATmega328P Microcontroller (Arduino Uno)
* TFT Display (ILI9341) - connected via SPI
* Analog Joystick (movement X/Y + push button for shield)
* Passive Buzzer (hit sound & game over music)
* RGB LED (visual feedback for player health)
