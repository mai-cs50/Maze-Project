The Maze
The Maze is a 3D maze game that uses raycasting to transform a 2D map into a fully navigable 3D world! The game is written in C
using the SDL2 library.

About SDL2
Simple DirectMedia Layer (SDL2) is a cross-platform development library that provides low-level access to audio, keyboard, mouse, joystick,
and graphics hardware through OpenGL and Direct3D. SDL2 is widely used in video playback software, emulators,
and popular games like those in Valve's award-winning catalog and many Humble Bundle games.

Installation
To install the project locally, clone the repository using the following command:

bash
Copy code
$ git clone https://github.com/mai-cs50/Maze-Project.git
Usage
To run the game after compiling, use the following command:

./maze or nake run

Controls:
Use the up and down arrow keys to move forward and backward (W and S serve the same function).
Use the left and right arrow keys to turn the camera around (A and D serve the same function).
Compilation
To compile the project, execute the following command:

gcc -Wall -Werror -Wextra -pedantic ./src/*.c -lm -o maze `sdl2-config --cflags` `sdl2-config --libs`
This ensures that the code adheres to strict C standards while utilizing SDL2.

Author
Mai
