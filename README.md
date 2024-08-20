# Nier Replicant ver.1.22474487139 Hardcore mode

This is a mod that makes NieR Replicant ver.1.22474487139 harder in interesting and fun ways.

## What does it do?

- Using an item requires and consumes a full bar of MP
- MP is slightly recovered on hitting enemies
- Passive MP recovery is slowed
- Player health is reduced
- MP is fixed to 100

Features can be adjusted and disabled easily, however that currently requires compiling from source.

## How to use it?

1. Back up save files (this mod shoudn't change them at all, but you should always do this for any mod)
2. Either download the precompiled files or compile yourself
3. Move `NieR_Replicant_Hardcore.dll` and `NieR_Replicant_Hardcore_launcher.exe` into your game installation folder. On steam, this is found by opening game page, clicking the gear and pressing `Browse Local Files`
4. Open `NieR_Replicant_Hardcore_launcher.exe` to play the game in hardcore mode. (you might need steam to be already open before this)
5. In game, set difficulty to Hard (You don't have to, if you want to try making it a little easier you could set it lower, but everything was tested with ingame difficulty on Hard)

If you want to stop using the mod for a bit, just load the game up any normal way you would. This mod shouldn't affect save files at all, so you can swap between using this mod or not as much as you like. To uninstall, just delete `NieR_Replicant_Hardcore.dll` and `NieR_Replicant_Hardcore_launcher.exe`

## Bugs / Issues

- There are small portions of the game where MP bar is not displayed. MP is still tracked, however this can make it difficult to see when you are able to use an item. These sections are short and easy so I don't see this as a huge problem.

## Troubleshooting

> Error when starting process. Error Message: The system cannot find the file specified.

> Error when launching process. Error message: DLL not found.

These errors imply it was not installed correctly. Make sure `NieR_Replicant_Hardcore.dll` and `NieR_Replicant_Hardcore_launcher.exe` are in the same folder as `NieR Replicant ver.1.22474487139.exe`

> Error when opening window handle. Error message: Handle is null.

Try making sure steam is already open before you open the launcher.

If you encounter any other issue or these steps don't fix it, then its my fault and iv made a mistake somewhere and id appreciate it if you let me know by commenting on the nexus page.

## Future additions / ideas

This is a list of ideas. Most of these will never be implemented (or if they will, they would be accompanied with a buff. The aim is to make the game hard in fun ways, not just make it crushingly tough), it's just a list from me brainstorming, il publish it here so if you have any feedback on them or suggestions id love to know.

- Make settings customisable without recompiling
- Limit health items to 3 each and make it so you automatically max out all 9 every time you go near a save point.
- Increase protaganists attack power to reduce the annoyance of bullet sponges
- Further limit MP, but increase magic damage
- Increase enemy spawn rates (not sure if this is possible for someone of my reverse-engineering skill to implement but it might be worth a try)
- Buff the effects of words
- Attack being blocked stuns the player
- Make item drops rarer
- Further lower health, to incentivise carful timing and awareness


## Compiling from source

Install vcpkg and either set the installation path to `VCPKG_ROOT` as an system-wide enivronment variable, or alter `CMakeLists.txt` to set the toolchain file path to your installation of vcpkg. Then run using CMake. This will install the required library Minhook automatically. If you build in the command line, make sure the terminal was opened as Administrator or if you run cmake through an ide like VS make sure it was opened as administrator