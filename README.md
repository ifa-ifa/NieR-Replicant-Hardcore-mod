# Nier Replicant ver.1.22474487139 Hardcore mode

Nier replicant has always had crappy difficulty scaling. The hard mode just makes enemies bullet sponges which doesn't actaully make it harder, it just makes it take longer to beat. For me, the big 3 problems are:
1) You can always just pause and spam heals whenever you are about to die
2) You can just run away and use long ranged magic
2) Bullet sponge enemies makes fights take foreveeeerrrrrrrr

These are the issues im attempting to fix with this mod.

## Mod functions


- Using an item requires and consumes a full bar of MP
- Player damage dealt increased 
- Player health is reduced
- MP is slightly recovered on hitting enemies
- Passive MP recovery is slowed

- MP is fixed to 100

Features can be adjusted and disabled using configuration file (requires game restart)

## Installation 

### Installation With Special K Local (Recommended)

Open the game once to cause the specialK configuration file to appear in the games install directory. (likely either dxgi.ini or d3d11.ini). Go to the end of that file and add this:

```
[Import.HardcoreMod]
Architecture=x64
Role=ThirdParty
When=PlugIn
Filename=NieR_Replicant_Hardcore.dll
```

Add `NieR_Replicant_Hardcore.dll` to the game installation foler.

### Installation With Special K Global

Start the game from the SpecialK launcher once to create the game profile. go to `SpecialKFolder/Profiles/NieR Replicant ver122474487139/`. Open `SpecialK.ini` and append this to the end of that file:

```
[Import.HardcoreMod]
Architecture=x64
Role=ThirdParty
When=PlugIn
Filename=NieR_Replicant_Hardcore.dll
```

Move `NieR_Replicant_Hardcore.dll` to that folder.

### Installation Without Special K

Go to the game installation folder and move `NieR_Replicant_Hardcore.dll` and `NieR_Replicant_Hardcore_launcher.exe` into it. To use the mod, launch from that exe. You can add it to steam by using the `Add a game -> Add a Non Steam Game` feature.

## Bugs / Issues

- Before meeting Weiss, the MP bar is not displayed. MP is still tracked, however this can make it difficult to see when you are able to use an item. These sections are short and easy so I don't see this as a huge problem.

## Troubleshooting

> Error when starting process. Error Message: The system cannot find the file specified.

> Error when launching process. Error message: DLL not found.

The mod may have been installed wrong. Try reinstalling the mod.

> Error when opening window handle. Error message: Handle is null.

Try making sure steam is already open before you open the launcher.

Any other issues, or these steps don't work, or if the mod just does nothing with no error, let me know on nexus or github

## Changelog

#### v1.0.1  
- Fixed bug
#### v1.0.2 
- Added configuration file, so no recompiling needed to change values. 
- Fixed bug where sometimes full MP bar was not required to use item.
- More robust injection
#### v1.1.0
- Heavily rebalanced, biggest change being large increase in player damage dealt.
- Uses better hooks which should make scaling more accurate and customising difficulty easier.
- Allows altering attack, magic attack, defense, magic defense
- No longer requires the launcher, instead using SpeicalK which many of you have installed. This will:
	- Fix steam integration
	- Require less files to be installed
	- Make using alongside other mods easier
	

## Future additions / ideas

This is a list of ideas. If you have any feedback on them or suggestions id love to know.

- Hot reloading settings
- Limit health items to 3 each and make it so you automatically max out all 9 every time you go near a save point.
- Increase protaganists attack power to reduce the annoyance of bullet sponges
- Further limit MP, but increase magic damage
- Increase enemy spawn rates. I can somewhat do this right now but chose not to implement it yet until i can find a way to make it good for proper gameplay. right now its so limited and buggy its just a funny gimick.
- Buff the effects of words
- Attack being blocked stuns the player
- Make item drops rarer
- Further lower health, to incentivise carful timing and awareness
