# Pong implementation

This is a pong implementation using SDL2.

AI, State Integration and timing need some love.

You can build 2 executables. One with static sreen-size
which is easier to review and a second version which scales in size - so you can play it in stamp size or UHD =)

AI-notes:  
Human rection time (RT) is about 200-300 ms max.  
Since this AI doesn't interpolate I cheated.  
RT could be ranged rand, like visibility could.  
But since this is no rocket science it's good for now.

Screenshot:  
![Screenshot](./screenshot.jpg)

## Building

Linking against: `-lSDL2_image -lSDL2_mixer -lSDL2_ttf`<br>

arch: `sudo pacman -S sdl2 sdl2_gfx sdl2_image sdl2_mixer sdl2_net sdl2_ttf`

On other Distros look out for and so on:<br>
deb: libsdl2,libsdl2-dev<br>
rpm: SDL2 SDL2-devel<br>

`make` will build native Linux executeables.<br>
`pong` has fixed screen-size.<br>
`pong_s` is the scaleable version.<br>

With `x86_64-w64-mingw32-gcc` installed `make pong_s_w64.exe` should build a Win64 version.<br>
You will need the runtime libs from https://www.libsdl.org/download-2.0.php to execute the windows version.

## Useful links:

[My Homepage](https://acry.github.io/)

[classic Pong and Breakout stuff](https://github.com/Acry/SDL2-pong_issue-01)  
[SDL2](https://www.libsdl.org/) [SDL-Discourse](https://discourse.libsdl.org) [More Example Code](https://gist.github.com/Acry/baa861b8e370c6eddbb18519c487d9d8)
