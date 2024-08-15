# 3DSCraft
Yet another revival of craftus, fourth generation, updating to more recent behavior and rebrandings.

The game itsself it shipped without assets, read how to extract them below:

## Index
 - [Playing](#playing)
   - [Default Controls](#default-controls)
   - [Building](#building)
 - [History](#history)
 - [Credits](#credits)

# Playing
> [!IMPORTANT]
> You need to *provide part of the assets for the game yourself*, otherwise the *game wont boot*. **You can not skip this step.**

> [!WARNING]
> **You are banned from sharing the assets around**, as we dont want to support cracking the game and getting it without pay.

You'll need a hacked system that can load homebrew for this to run
### Check out the [Resource Packer](https://github.com/Team8Omega/3DSCraft-ResourcePacker).

## Default Controls

You can change following controls in `sd:/3dscraft/options.ini`:

| ***Action*** | ***OLD 3DS*** | ***NEW 3DS*** |
| ------------- | ------------- | ------------- |
| **Place Block** | L | L |
|  **Break Block** | R | R |
|  |  |  |
| **Walk Forward** | Circle Pad (Up) | Circle Pad (Up) |
| **Walk Backward** | Circle Pad (Down) | Circle Pad (Down) |
| **Strave Left** | Circle Pad (Left) | Circle Pad (Left) |
| **Strave Right** | Circle Pad (Right) | Circle Pad (Right) |
| **Jump** | DPad (Up) | A |
| **Sneak** | DPad (Down) | B |
|  |  |  |
| **Camera Up** | X | C-Stick (Up) |
| **Camera Down** | B | C-Stick (Down) |
| **Camera Left** | Y | C-Stick (Left) |
| **Camera Right** | A | C-Stick (Right) |
|  |  |  |
| **Quickselect Left**| None | LZ |
| **Quickselect Right** | None | RZ |
|  |  |  |
| **Open Console** | Select | Select |
|  |  |  |
| **Pause** | Start | Start |

## Building
- Install Devkitpro
- Install libraries as follows
   - ```pacman -S 3ds-libvorbisidec 3ds-pkg-config```
- run make, read the errors properly.
- Get 3DSCraft.cia/.3dsx :)

# History
 - [Our repo here](https://github.com/Team8Omega/3DSCraft) is a [reupload of](https://github.com/EinfachEli08/3DSCraft) the fork from
 - ["Modification of Craftus Redesigned"](https://github.com/Onixiya/craftus) by Onixiya, which forked 
 - [Craftus Redesigned](https://github.com/yunes-0840/Craftus-Redesigned) by GamesSanti/yunes-0840, which forked 
 - [Craftus_Reloaded](https://github.com/RSDuck/craftus_reloaded) by RSDuck, which forked the original 
 - [Craftus3DS](https://github.com/RSDuck/Craftus3DS) by RSDuck.

# Credits
* ag_0815 for fixing audio
* People in the nintendo homebrew discord server willing to answer stupid questions
* KosmicDev for better y spawning code
* Onixiya who modified Craftus Redesigned(fork)
* GamesSanti who made Craftus Redesigned
* RSDuck who made Craftus and Craftus Reloaded
* Contributors who Forked Craftus and contibuted their changes each time
* People who made 3DS homebrew possible
    * Especially smea for 3dscraft which is good place for ~~stealing ideas~~ inspiration
    * Contributors of the 3dbrew.org wiki and ctrulib
    * Fincs for citro3d
    * More people I forgot about, even though I extended this list multiple times
* Tommaso Checchi, for his [culling algorithm](https://tomcc.github.io/2014/08/31/visibility-1.html), orginally developed for the use in MC PE
* Of course all other MC devs, how could I forget them?
* All people who worked on the libraries Craftus is depending on(see the lib folder)