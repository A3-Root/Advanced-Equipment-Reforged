# Advanced Equipment Reforged (AER)

Interactive laptops, servers, routers, flash drives, cables and an electrical grid for Arma Reforger. Port of [Advanced Equipment Revamped](https://github.com/y0014984/Advanced-Equipment) (Arma 3).

A laptop runs armaOS: a terminal (command line), a desktop (windows and apps), or both. Computers have users, passwords, a permission-based filesystem, mail, chat, web pages, a calendar, CCTV, games and network tools. Routers build wireless and cabled networks; generators, batteries and solar panels power the devices through cables players carry and plug in. Game Masters and mission makers add intel, accounts and links in game, in the World Editor or by script.

## Requirements

- Arma Reforger 1.9 or later.

## Installation

Subscribe to the addon in the Arma Reforger Workshop and enable it for the scenario, or add it to the server `config.json` mods list. To build from source, open `addon.gproj` in Arma Reforger Tools.

## What it adds

### Equipment

| Item | Notes |
| --- | --- |
| Laptop | Computer with lid, internal battery, 2 USB ports, screen on the model; carried (2x2) and used from the inventory |
| Portable server | Headless computer on mains power, remote access (SSH) on |
| Router | Wireless range 100 m, DHCP, password, cascades to other routers by cable |
| Flash drive | Own filesystem (8 GB), plugs into a USB port and shows in the port |
| Network cable | 5 m, 15 m, 30 m items; plugged in two steps and drawn between the devices |
| Battery pack, solar panel | Store and supply power; the solar panel follows the sun |
| Generators | Five vanilla generators with fuel tanks, refuelled from a carried jerry can |
| Floodlight | Lamp with a built-in generator (10 L, 0.5 L/h) or an external power cable |

Laptop, router, battery pack, solar panel and portable server come in Black, Olive and Sand; flash drives in red,
blue and yellow. All are in the Game Master Edit list, the World Editor and the US, USSR, FIA and CIV arsenals.

### Laptops: terminal and desktop

- armaOS terminal with login, `sudo` / `su`, redirection, Tab completion, history, 20 designs and 46 programs (8 of them network games).
- Desktop in two styles per computer: civilian (Windows look) and military (Tactical look), each with three themes.
- 21 desktop apps: My Computer, Files, Notepad, Photos, Terminal, Browser, Email, Messenger, Calendar, Network,
  Remote Access, Ping, Map, CCTV, Games, Snake, Calculator, Recycle Bin, Settings, System Info, About.
- Taskbar with search for apps and files, tray (flash drive, network, power) and a quick panel.
- Sounds for start, shut down, standby, flash drives, errors, mail and messages.
- Interface per computer: terminal only, desktop only or both; access lists by identity, faction or player name.

### Intel

- Files and folders with owner / everyone permissions, password-locked files and 15 ciphers (Caesar to Ascii85).
- Mail, chat handles, web pages and sites, browser history, images and videos, calendar events, CCTV cameras.
- `crypto` and `crack` programs for players to decrypt intel.

### Power and network

- Power roles: consumer, battery, generator, solar panel; overload, internal batteries, mains power.
- Routers with SSID, password, range, static IPs, SSH filters; ping, SSH, remote file access, network messages.
- Power and network cables plugged by players (cable length checked), by the Game Master or the World Editor.

### Game Master and World Editor

- Action panel with 43 actions (users, files and intel, power, network, desktop style, save / restore).
- Device attributes and an AE3 System module for mission-wide settings.
- Components with every device setting for the World Editor; settings in the mission header.

### Script API

`ROOT_AE3_API` adds users, files, mail, web pages, links and power states, registers custom programs and desktop apps, and raises events for logins, commands, file changes and device setup.

## Documentation

The full documentation is in the [wiki](wiki/Home.md).

- [Getting Started](wiki/Getting-Started.md)
- [Player Guide](wiki/Player-Guide.md)
- [Game Master Guide](wiki/Game-Master-Guide.md)
- [World Editor Guide](wiki/World-Editor-Guide.md)
- [Server Admin Guide](wiki/Server-Admin-Guide.md)
- [Script API](wiki/Reference/API.md)

## Reporting issues

Use the [issue tracker](https://github.com/A3-Root/Advanced-Equipment-Reforged/issues). Include the addon version, the game version, singleplayer / hosted / dedicated, the loaded addons, steps to reproduce and the relevant part of `console.log` / `script.log`.

## Contributing

Pull requests are welcome. Keep changes to one fix or feature and update the wiki and `releases/CHANGELOG.md` (one line per change) with them. Before submitting, the project must compile without script errors or warnings in Arma Reforger Tools (Workbench). 

## Credits

A3-Root (a. xMidnightSnowx / Root)

## License

Arma Public License Share Alike. See [LICENSE](LICENSE).
