# 3DSCraft
Yet another revival of craftus, updating to more recent behavior and rebrandings.

## Building
- Install Devkitpro
- run make, read the errors properly.
- Get 3DSCraft.cia/.3dsx :)

You'll need a hacked system that can load homebrew for this to run (citra is good for testing but i haven't been able to get most of the controls working there)

We moved our old repo to this fork, to keep the long history of craftus intact, but noticed that the history wasnt kept on this fork...
https://github.com/EinfachEli08/3DSCraft (old repo)

This was forked from Onixiya/craftus: https://github.com/Onixiya/craftus


After launching it for the first time, you can change the controls in `sd:/3dscraft/options.ini`.
You can also change the settings in `sd:/3dscraft/options.ini`.

## Controls

| Category | Action | OLD 3DS | NEW 3DS |
| ------------- | ------------- | ------------- | ------------- |
| Blocks | Place BlockId | L | ZL |
| Blocks | Break BlockId | R | ZR |
|  |  |  |  |
| Move | Forward | Circle Pad (Up) | Circle Pad (Up) |
| Move | Backward | Circle Pad (Down) | Circle Pad (Down) |
| Move | Left | Circle Pad (Left) | Circle Pad (Left) |
| Move | Right | Circle Pad (Right) | Circle Pad (Right) |
| Move | Jump | DPad (Up) | A/B/DPad (Up) |
| Move | Shift | DPad (Down) | Y/X/DPad (Down) |
|  |  |  |  |
| Camera | Up | X | C-Stick (Up) |
| Camera | Down | B | C-Stick (Down) |
| Camera | Left | Y | C-Stick (Left) |
| Camera | Right | A | C-Stick (Right) |
|  |  |  |  |
| Inventory | Switch BlockId | DPad (Left) | L/DPad (Left) |
| Inventory | Switch BlockId | DPad (Right) | R/DPad (Right) |
|  |  |  |  |
| Commands | Open Console | Select | Select |
|  |  |  |  |
| Menu | Back | Start | Start |


# Credits
* ag_0815 for fixing audio
* People in the nintendo homebrew discord server willing to answer stupid questions
* KosmicDev for better y spawning code
* GamesSanti who made Craftus Redesigned
* RSDuck who made Craftus and Craftus Reloaded
* People who made 3DS homebrew possible
    * Especially smea for 3dscraft which is good place for ~~stealing ideas~~ inspiration
    * Contributors of the 3dbrew.org wiki and ctrulib
    * Fincs for citro3d
    * More people I forgot about, even though I extended this list multiple times
* Tommaso Checchi, for his [culling algorithm](https://tomcc.github.io/2014/08/31/visibility-1.html), orginally developed for the use in MC PE
* Of course all other MC devs, how could I forget them?
* All people who worked on the libraries Craftus is depending on(see the lib folder)
