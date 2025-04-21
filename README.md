# Nier Replicant ver.1.22474487139 Hardcore mode

## Mod functions

- Using an item requires and consumes a full bar of MP
- MP is slightly recovered on hitting enemies
- Passive MP recovery is slowed
- Player health is reduced
- MP is fixed to 100

Features can be adjusted and disabled using configuration file (requires game restart)

## How to use

1. Back up save files (this mod shoudn't change them at all, but you should always do this for any mod)
2. Move `NieR_Replicant_Hardcore.dll` and `NieR_Replicant_Hardcore_launcher.exe` into your game installation folder. (TO find this on steam, open game page, click the gear and press `Browse Local Files`)
3. optionally, add `NieR_Replicant_Hardcore.ini`. (if you don't it should just use default settings)
4. Open `NieR_Replicant_Hardcore_launcher.exe` to play the game in hardcore mode.
5. In game, set difficulty to Hard (You don't have to, if you want to try making it a little easier you could set it lower, but everything was tested with ingame difficulty on Hard)

If you want to temporarily stop using the mod, just load the game up any normal way you would. To uninstall, just delete `NieR_Replicant_Hardcore.dll` and `NieR_Replicant_Hardcore_launcher.exe`

## Bugs / Issues

- There are small portions of the game where MP bar is not displayed. MP is still tracked, however this can make it difficult to see when you are able to use an item. These sections are short and easy so I don't see this as a huge problem.

## Troubleshooting

> Error when starting process. Error Message: The system cannot find the file specified.

> Error when launching process. Error message: DLL not found.

Make sure `NieR_Replicant_Hardcore.dll` and `NieR_Replicant_Hardcore_launcher.exe` are in the same folder as `NieR Replicant ver.1.22474487139.exe`

> Error when opening window handle. Error message: Handle is null.

Try making sure steam is already open before you open the launcher.

If you encounter any other issue then its probably my fault and i wouldd appreciate it if you let me know by commenting on the nexus page.


## Changelog

#### v1.0.1  
- Fixed bug
#### v1.0.2 
- Added configuration file, so no recompiling needed to change values. 
- Fixed bug where sometimes full MP bar was not required to use item.
- More robust injection







## Future additions / ideas

This is a list of ideas. Most of these will never be implemented (or if they will, they would be accompanied with a buff. The aim is to make the game hard in fun ways, not just make it crushingly tough), if you have any feedback on them or suggestions id love to know.

- changing settings does not require restart
- Limit health items to 3 each and make it so you automatically max out all 9 every time you go near a save point.
- Increase protaganists attack power to reduce the annoyance of bullet sponges
- Further limit MP, but increase magic damage
- Increase enemy spawn rates (not sure if this is possible for someone of my reverse-engineering skill to implement but it might be worth a try)
- Buff the effects of words
- Attack being blocked stuns the player
- Make item drops rarer
- Further lower health, to incentivise carful timing and awareness
