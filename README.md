# Advanced Equipment Reforged

Hackable laptops, servers, routers, flash drives and an electrical grid for Arma Reforger. Port of Advanced
Equipment (Arma 3) 2.1.0.8. No dependencies besides Arma Reforger.

## Devices

| Prefab | Role |
| --- | --- |
| `Prefabs/ROOT_AE3/Devices/ROOT_AE3_Laptop.et` | Computer, inventory item (2x2, 2.2 kg), 2 USB ports, screen on the model, lid, internal battery (0.1 kWh, 10 W on, 0.1 W standby) |
| `Prefabs/ROOT_AE3/Devices/ROOT_AE3_PortableServer.et` | Headless computer on mains (150 W), SSH enabled, inventory item (3x2, 25 kg) |
| `Prefabs/ROOT_AE3/Devices/ROOT_AE3_Router.et` | Router, inventory item (2x1, 0.8 kg), 100 m wireless range, internal battery |
| `Prefabs/ROOT_AE3/Devices/ROOT_AE3_UsbStick.et` | Flash drive with its own filesystem, inventory item (1x1, 0.02 kg) |
| `Prefabs/ROOT_AE3/Devices/ROOT_AE3_BatteryPack.et` | Battery 0.6 kWh, recharges at 300 W, inventory item (2x2, 12 kg) |
| `Prefabs/ROOT_AE3/Devices/ROOT_AE3_SolarPanel.et` | 150 W peak, output by sun elevation and panel tilt, 0.4 kWh buffer |
| `Prefabs/ROOT_AE3/Power/ROOT_AE3_Generator_*.et` | Generators on vanilla models: portable 5 kW / 5 L / 1.5 L/h, military and airfield 100-400 kW |
| `Prefabs/ROOT_AE3/Power/ROOT_AE3_Floodlight.et` | Lamp, 150 W, lit while powered |

Solar panels switch on and off like generators. Flash drives take files from the World Editor
(`ROOT_AE3_UsbDriveComponent` files), the Game Master (add file / directory / image) and the script API.
Laptop, router, battery pack, solar panel and portable server also come in Olive, Sand and Black
(`Prefabs/ROOT_AE3/Devices/Variants/`). All devices are placeable in Game Master (Edit mode) and in the World Editor. Laptops, routers, battery packs, flash drives and portable servers are in
the US, USSR, FIA and CIV arsenals (equipment). Device names are in the string table `Language/ROOT_AE3_localization.st` (English) and
searchable in the Game Master browser; preview images are in `UI/Textures/EditorPreviews/ROOT_AE3/`. Inventory items fit uniform, vest and backpack storage.
The 3x2 inventory slot is added by `Scripts/Game/ROOT_AE3/Inventory/ROOT_AE3_SlotSize.c`.

## Player actions

Use terminal, Use desktop (place a laptop to use it), Power on / Shut down, Standby / Wake up, Open / Close lid, Plug into power source
(6 m), Plug network cable into router (10 m), Insert (first drive in the inventory) / Eject (highest occupied port; the desktop Volumes app ejects any port) flash drive, Refuel (drains a carried fuel container,
such as a jerry can),
Check status. The server checks that the player is alive and within 5 m for actions; sessions start within 4 m.
Laptop, router, battery and flash drive are picked up and placed like other items; picking up a laptop ends
the session, ejects drives, unplugs power and puts it into standby.

## armaOS terminal

Login (`LOGIN>` / `PASSWORD>`), prompt `user@host:/path/>`, quotes, `>` / `>>` redirection, Tab completion,
history (Up / Down arrows), Ctrl+C button, 20 terminal designs, font size, mouse wheel scrolling.

Programs: `man help ls cd cat date history clear rm mv cp whoami mkdir touch echo find grep mount umount lsusb
chown unlock sudo su desktop ping ip ifconfig wifi ssh msg exit shutdown standby`, optional `crypto crack`
(component attribute or `InstallCommand`), `snake` and the network games
`chess checkers uno ludo snakesladders blackjack rps battleship`. Programs are files (`#!ae3 <name>`) in
`/bin`, `/sbin`, `/usr/bin`, `/games`; a program runs only when its root-owned file exists and it was installed on that
computer, so deleting the file removes it. Program files on flash drives or outside these folders do not run. Other executable files run as scripts
(one command per line).

Root login is off by default; users in `/etc/sudoers` use `sudo` / `su`. While `m_bCliElevateSudoers` is on
(default), sudoers act as root on the filesystem; the account `admin` always does and counts as a sudoer. Program files (`#!ae3 <name>`) run only when owned by root. Default root password `toor`. Failed logins and unlocks go to
`/var/log/auth.log`.

## Desktop

Sign-in screen, plain black background, wallpapers 1-5, an image file or a web address, themes (Dark, Light, Olive), windows (drag, snap to the top or a side edge, resize
from the corner), taskbar with pinned apps, start menu with search, right-click menus, dialogs, Open / Save file
dialog, notifications.
Apps: My Computer, Files, Notepad, Photos, Terminal, Browser, Email, Messenger, Calendar, Network,
Remote Access, Ping, Map, CCTV, Games, Snake, Calculator, Recycle Bin, Settings, System Info, About. Other mods add apps
with `ROOT_AE3_API.RegisterExtApp`; their server requests are handled through
`ROOT_AE3_API.RegisterGuiCommand` (subclass of `ROOT_AE3_GuiCommand`).

## Power and network

Generators and computers play their running sound while on. Internal batteries and solar buffers deliver up to
`m_fBufferOutputW` (default 1000 W). Consumers draw from one provider; batteries charge from a provider and supply their own consumers; an
overloaded generator or battery switches off; unsupplied devices run on their internal battery and switch off
when it is empty. Routers get `192.168.N.1/24`, hand out addresses (DHCP or static IP inside the subnet),
cascade by cable, accept wireless clients in range with an optional password and filter external SSH by
regular expressions. Each computer gets the mail address `<hostname>@lan` and the chat handle `@<hostname>` when it
has none; users add and delete further mail addresses (16 per computer) and chat handles (8 per computer) in Email and Messenger. `ping`, `ssh`, `msg`, mail and chat need a powered path between both devices. Mail and chat belong
to the computer: every logged-in user reads its mail folders and chat logs. A network cable needs physical access and skips
the router password; it needs a router that accepts connections.

## Game Master

Context action "Advanced Equipment..." on hovered or selected devices opens a panel of the actions that apply,
grouped Users, System, Files and intel, Power, Network: add user / sudoer, remove sudoer, root password, root
login, hostname, interface mode, add file (permissions, Caesar / columnar encryption, password, virtual size),
add directory, add email, mail address, chat handle, add web page, browser history, image, calendar event,
install games, static IP, interface access (identity IDs / factions / names), user transfer speed, transfer speed, battery level, fuel, router settings, power on / off, standby, crash, connect
power / network (select the source or router with the devices), disconnect, open interface as root, save /
restore computer (empty name lists the saves). Text values are entered in a form; `
` in file, mail, web page and calendar text is a line break.
Each action shows a notification with the number of devices it changed. "Open interface as root"
opens the desktop as root for the Game Master without range or access checks (filesystem browser); the previous
login is restored when the Game Master closes it.

Device attributes (category "Advanced Equipment"): powered on, battery level, fuel, root login, interface mode,
SSH server, router range, accept connections, external SSH. Game settings (category "Advanced Equipment"):
boot and shutdown time, root login at terminal, sudoers act as root, any login, log out on session end,
default interface, desktop theme, window size, file browsing, screen on models, router range and multiplier,
network transfer speed, games. Game setting changes are sent to all players.
Attributes: `Configs/Editor/AttributeLists/Edit.conf`, `Scripts/Game/ROOT_AE3/Editor/ROOT_AE3_EditorAttributes.c`.

## World Editor

| Component | Attributes |
| --- | --- |
| `ROOT_AE3_ComputerComponent` | interface mode, terminal / desktop access (identity IDs, faction keys, player names), hostname, root login, root password, users (name, password, sudoer), sudoers, install commands / games / crypto and crack, USB ports |
| `ROOT_AE3_SetupComponent` | files and directories, mail addresses, mails, chat handle, web pages, browser history, calendar (with location), wallpaper (0 black, 1-5) |
| `ROOT_AE3_PowerComponent` | role, draw on / standby, output, capacity, recharge, fuel tank, burn rate, mains, external only, on at start, start level, start fuel, buffer output, solar panel tilt |
| `ROOT_AE3_NetworkComponent` | router: SSID, gateway, range, multiplier, accept clients, password, external SSH, allow list, speed; client: static IP, speed, SSH |
| `ROOT_AE3_ConnectionComponent` | power source and router by entity name (cable links) |
| `ROOT_AE3_DeviceComponent` | type, name |

## Settings

Defaults in `Configs/ROOT_AE3/Settings.conf`. A mission overrides them with `m_ROOT_AE3_Settings` in its
`SCR_MissionHeader`. The Game Master changes part of them in game (see Game Master).
Groups: Main (debug log, network debug log, any-login test mode, device names), armaOS (root login, sudoers, root password, error sound, transfer speeds
and cap, terminal design, scroll, font size, screen mirror range / interval / viewers / lines / keystroke sync
/ change detection, header texts, boot and shutdown time), Desktop (default interface, file
browsing, theme, map same network, window size, web access), Network (range, multiplier), Power (sync threshold, sync, interval), Games
(enabled, hosting, same network, max tables, turn and abandon timeout, power draw, idle draw,
practice seats, bots).

## Script API

`ROOT_AE3_API` (authority only): `AddUser`, `AddSudoer`, `RemoveSudoer`, `SetRootPassword`, `SetRootLogin`,
`SetHostname`, `AddFile`, `AddDirectory`, `ReadFile`, `RemovePath`, `AddMedia`, `AddCustomCommand`,
`InstallCommand`, `AddMailAddress`, `AddEmail`, `SetChatHandle`, `RegisterWebpage`, `RegisterSite`,
`AddBrowserHistory`, `AddCalendarEvent`, `RemoveCalendarEvent`, `RegisterCamera`, `AddGames`, `HostGameWithBots`, `ConnectPower`,
`DisconnectPower`, `ConnectNetwork`, `DisconnectNetwork`, `SetPowerState`, `CrashDevice`, `SetBatteryLevel`,
`SetFuel`, `SetTransferSpeed`, `GetAddress`, `SaveComputer`, `RestoreComputer`, `RegisterGuiCommand`, `SetInterfaceMode`, `SetInterfaceAccess`, `SetStaticIp`, `SetMainsPowered`,
`GetPowerState`, `GetBatteryLevel`, `GetFuel`, `AddAccessCondition` / `RemoveAccessCondition` (`ROOT_AE3_AccessCondition`),
`AddConnectCondition` / `RemoveConnectCondition`
(`ROOT_AE3_ConnectCondition`), `AddRangeModifier` / `RemoveRangeModifier` (`ROOT_AE3_RangeModifier`); client:
`RegisterExtApp`.
Events: `GetOnLogin`, `GetOnCommand`, `GetOnFileChanged`, `GetOnDeviceReady`.

## Locality

| | SP / listen host | Dedicated server | Client |
| --- | --- | --- | --- |
| Filesystem, users, shell, mail, games | local authority | authority | none (requests by RPC) |
| Power and network simulation | local authority | authority | replicated state only |
| Terminal and desktop UI | local | none | local menu |
| Screen on the model | rendered | not rendered | rendered within range |

Client requests go through `SCR_PlayerController` RPCs; only the session owner receives its output. Screen
text goes by RPC only to players within the screen mirror range, up to the viewer setting per device;
password arguments of `ssh`, `unlock`, `wifi connect` and `crypto -k` are masked on the screen.

## Persistence

The vanilla persistence system (`Configs/Systems/Persistence/Configuration/ROOT_AE3_Devices.conf`) saves every
device prefab: armaOS state (filesystem, users, history, preferences, calendar, mail addresses, own web pages,
plugged flash drives and their mounts, lid, access lists), power state, battery level, fuel, flash drive content, power and network links, and
router / client network settings. Web pages for every computer come from the World Editor setup again; ones added by the Game Master or the
API are not saved. Game
Master save / restore writes `$profile:ROOT_AE3/saves/<name>.txt`.

## Access and range

Sessions start within 4 m of the computer and end when the player moves away, dies or loses the character.
The logged-in account stays after a session ends unless `m_bLogoutOnSessionEnd` is on (log out with `exit` or
Sign out); `su`, SSH and running
programs end with the session. Changing the terminal design for the computer needs a sudoer.
Terminal and desktop requests are accepted only for the open interface and the access lists of the computer.
Access list entries are player identity IDs, faction keys or player names (names can be changed by players).
Failed passwords delay further attempts of that player by 1 s per failure, up to 5 s. Each filesystem stores up
to 500,000 characters, a file at most 65,536; flash drives also hold at most their capacity. Log files over
16,000 characters are cut to their newest 8,000. `grep` and `find` match the first 1,024 characters of each line; terminal lines keep
1,024 characters. With desktop file browsing off, the desktop opens and saves only files inside the user's home folder.
Each command prints at most 65,536 characters and folder listings show 500 entries. Each player receives at
most 65,536 characters per second plus the message that crosses it; later messages in that second are
dropped. On a router without password only sudoers change its settings. `/.trash` is root-only; trash entries are
restored or deleted by their owner or root through the Files app. Mail folders keep 100 mails, a computer
sends at most one mail every 2 s; a filesystem holds at most 5,000 files, folders and links. Scripts run at
most 50 lines. Failed player actions show the reason as a hint. Destroyed devices switch off and offer no actions. Static IP,
SSH access and router settings are set by the Game Master, World Editor or script API only; the desktop Network app joins
and leaves networks. Dropped terminal output is resent as the last 100 lines.

## Not ported

Music app, the debug overlay, translations (the interface and string table are English only), the full desktop on the model screen (it shows the window titles), CCTV of cameras outside the
viewer's streaming range, Game Master action for multi-page web sites (script API `RegisterSite` only), the pixel canvas API for other mods (Arma 3 `fnc_retro_*`), naming a laptop when it is placed, removing installed games, the Zeus games roster and per-computer
games module, file drag and drop, graphical game boards (the desktop Games app shows boards as text), the terrain map in the Map app (it shows devices on a grid), copying and exporting mail and chat, portable desks and desk chairs, lamp variants (one floodlight), animated GIF, video formats the engine video player does not take, page styles and scripts in the Browser, typing
animations, refuelling generators from fuel trucks or nozzles, the Arma 3 deployment type setting (Reforger
placeable items).
