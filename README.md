# Heroes of Newerth ZoomHack (Linux only)

I sometimes still play HoN mid wars. I think that full zoom should be allowed by default in the game. 

Using my reverse engineering skills I found parts in assembly that are responsible for the camera zoom and edited them to remove all limits.

**IMPORTANT! Your camera zoom is still sent to server! So admins will see that you use zoom hack! Use at your own risk**

![Showcase](showcase.png)

## Prerequisites 

 * Clang 7+

## Usage
* Build `./build.sh`
* Run `sudo ./zoomHack` when game is opened

## TODO
* Automatically get needed export function locations
* Remove the fog effect for cleaner view