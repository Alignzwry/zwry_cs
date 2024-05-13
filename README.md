# zwry_cs
Counterstrike 2 External Kernel Cheat

# Build:
set the build mode on all projects to Release!
To compile the driver, you will first need to go to Properties-->Driver signing-->General-->Test Certificate-->"Create Test Certificate", you may need to give Visual studio admin rights for that.
you dont need to compile start.exe, you can also use create.cmd every time you recompile the driver if you dont want start.exe

# Usage:
Make sure all the compiled files are in one folder!
run zwry_esp.exe <br />
use delete.cmd to unload the driver <br />

Note: If you dont use create.cmd, the program will load the driver itself using start.exe, but it will close itself first so you will need to reopen it!

# credits:
inspo and help: https://github.com/CowNowK/AimStar <br />
Kernel Driver: https://github.com/mbn-code/The-Kernel-Driver-Guide-External

# useful:
for disabling signature enforcement: https://github.com/Mattiwatti/EfiGuard

# ToDo:
- add a menu
- more error handling
- performance optimization
