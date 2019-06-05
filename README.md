# NARC

## "Not Another RayCaster"

A raycasting game engine in C++, built using my own library lwmf, OpenGL and SDL 2.0.

I am working on a 2.5D, "Wolfenstein 3D"-like game engine, written in modern C++.

Currently I´m developing in Visual Studio 2019, for Windows as the target platform.

The project began with GDI+ and pure software rendering, but I switched to OpenGL for better handling and performance.

There is a long way to go with the engine, from entity AI to weapon handling, level editor etc. The project is still at the very beginning, although the render engine itself is stable and powerful, but lacking features, which will come in the near future.

The whole engine is modular and written in C++ v17.

Due to (partial) use of SIMD Intrinsics a CPU with SSE 4.2 support is needed (virtually all Intel CPUs from 2007 on).

The demoproject resembles a "between Wolfenstein 3D and Doom" retro style: Textures are 64x64 in size, everything is pixelated, but you can also use crytal-clear hires textures...

**If you want to check it out without building it by yourself, just download "Release.zip", unzip and run the exe. Everything you need is included!**

**Current features :**

  - hardware accelerated by native OpenGL 4.5 Core (Shader, not immediate/direct mode!)
  - own OpenGL wrapper - no need for glew, glad & Co. !
  - windowed or fullscreen
  - vsync by option (vsync implies fullscreen mode!)
  - changed options will be saved
  - indoor & outdoor levels (ceilings and/or skybox, lighting or not)
  - you can shoot entities and they will vanish after they lose their hitpoints
  - entities will attack you - and you can die...DIE...
  - pixel-precise detection of hits on enemies
  - A* pathfinding algorithm (started, path between entities and player is calculated)
  - pickup ammo boxes and gain new ammo
  - variable texture size (from 64x64 to 2048x2048) for level textures and entities
  - entities and weapons implemented as structures
  - really large map size if desired
  - maps, weapons and entities can be defined in textfiles which will be loaded into program at runtime
  - complete separation of content/config from code
  - fixed timestep gameloop with 60fps (or chooseable) framelock
  - multidirectional, animated or non-animated entities with basic automovement
  - animated doors (in work, but you can open them and they will close automatically, as there is sound)
  - multi-threaded rendering of graphics with dynamic resolving of concurrent threads and use of a threadpool
  - collision detection between walls, entities and player
  - per-pixel softwareshading for viewsize-dependent darkening of environment
  - real time lighting (somewhat faked, needs more work)
  - movement via free mouselook (x- and y-axis) and wasd (including strafing)...
  - ...or just use a connected gamepad like a XBOX 360 Controller. Gamepads will be detected automatically!
  - adjustable input sensitivity
  - basic audio functions (background music, steps, weapon sounds, entity sounds, door sounds)
  - basic weapon behaviour (sway, muzzleflash, shooting, reloading, ammo pickup, change weapon)
  - realtime minimap showing entity positions
  - basic implementation of HUD (weapon, healthbar and ammo info) / Crosshair
  - fps (frames per second) counter
  - error handling, file checks
  - handling of INI files (reading/writing values, cross-platform)
  - basic implementation of main menu, options
  - autoupdates via web/HTTPS for e.g. "gamecontrollerdb.txt" (using libcurl)
  - really fast & lightweight text rendering via stb_truetype.h and pre-generated glyph textures (OpenGL)

![NARC_PIC1](https://github.com/StefanKubsch/NARC/blob/master/Documentation/NARC.png)

![NARC_PIC2](https://github.com/StefanKubsch/NARC/blob/master/Documentation/NARC1.png)

**Used libraries/APIs:**

  - lwmf, the lightweight media framework for graphics, multithreading, input handling, logging etc. (my own work, have a look here: https://github.com/StefanKubsch/lwmf)
  - SDL 2.0.9 for low level handling of audio and gamepad support (https://www.libsdl.org/)
  - SDL_Mixer 2.0.4 for audio handling (https://www.libsdl.org/projects/SDL_mixer/)
  - stb_truetype.h 1.21 for TrueType font rendering (https://github.com/nothings/stb)
  - libcurl 7.65.1 for internet handling (https://curl.haxx.se/libcurl/, see documentation in Docfolder!)
  - {fmt} 5.3.0 for printing and formatting strings etc. (https://github.com/fmtlib/fmt)
  
**Controls (configurable):**
  
	Gamepads are supported
	
	Left analog stick 	- forward, backward, strafe left, strafe right
	Right analog stick 	- look left/right/up/down
	Right shoulder button 	- fire single shot
	Left shoulder button	- rapid fire
	X			- open doors when standing in front of them
	DPAD Up/Down 		- change weapon
	DPAD Right 		- reload weapon
	
	---
	
	Mouse 			- look left/right/up/down
	Left mouse button 	- fire single shot
	Right mouse button	- rapid fire
	Mousewheel up/down 	- change weapon
	
	'W' 	- forward
	'S' 	- backward
	'A' 	- strafe left
	'D' 	- strafe right
	'R' 	- reload weapon
	'Space' - open doors when standing in front of them
  
	'H' - toggle HUD
	'M' - toggle Minimap
	'+' - increase mouse sensitivity
	'-' - decrease mouse sensitivity
	
	'N' - switch level ingame (skip to next level or first if last level is reached)
	'L' - switch lighting on/off
	
	'ESC' - pause game / break into menu / return to game
	
	'Cursor Up' & 'Cursor Down' & 'Return' - navigate through menu
