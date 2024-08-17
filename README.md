Build m1 Simulator with:

gcc starfield.c -o starfield -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lm

./starfield


Momentum Firmware Build:

gcc -o starfield starfield.c -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2

./starfield
