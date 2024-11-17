Build m1 Simulator with:

gcc starfield.c -o starfield -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lm

./starfield


Momentum Firmware Build:

ufbt update --index-url https://up.momentum-fw.dev/firmware/directory.json

ufbt launch APPSRC=starfield 
