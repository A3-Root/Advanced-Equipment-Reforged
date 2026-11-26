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
Laptop, router, battery pack, solar panel and portable server are placed and issued as Black (orange accents), Olive
(coyote accents) and Sand (ranger green accents) variants (`Prefabs/ROOT_AE3/Devices/Variants/`): body colour plus accent
side bands, lid plate, bumpers, router and battery bands, solar panel frame and rim, server top plate. The base prefabs
are their parents only and are not placeable or issued. Flash drives come in red, blue and yellow. Computers and routers have a green power and a blue
network LED. All devices are placeable in Game Master (Edit mode) and in the World Editor. Laptops, routers, battery packs, flash drives and portable servers are in
the US, USSR, FIA and CIV arsenals (equipment). Device names are in the string table `Language/ROOT_AE3_localization.st` (English) and
searchable in the Game Master browser; preview images are in `UI/Textures/EditorPreviews/ROOT_AE3/`. Inventory items fit uniform, vest and backpack storage.
The 3x2 inventory slot is added by `Scripts/Game/ROOT_AE3/Inventory/ROOT_AE3_SlotSize.c`.

## Player actions

Use terminal, Use desktop (place a laptop to use it), Power on / Shut down, Standby / Wake up, Open / Close lid, Plug into power source
(6 m), Plug network cable into router (10 m), Insert (first drive in the inventory) / Eject (highest occupied port; the desktop Volumes app ejects any port) flash drive, Refuel (drains a carried fuel container,
such as a jerry can),
Check status, Rename. The server checks that the player is alive and within 5 m for actions; sessions start within 4 m.
Laptop, router, battery and flash drive are picked up and placed like other items. Picking up a laptop keeps its
flash drives plugged in (shown again when it is placed), unplugs power, ends other players' sessions and puts it
into standby. Inventory "Use" (mouse, keyboard, gamepad) on a laptop in the own inventory (uniform, vest, backpack; on
foot or in a vehicle) shows its name field with Open and Rename. Open wakes or boots it and opens the desktop (terminal
for terminal-only computers); the session lasts while the laptop stays in that inventory and does not mirror its
screen to other players. When the session ends, a carried laptop goes to standby. Carried laptops use wireless
networks in range of the carrier.
The inventory shows a device's name ("<name> (Laptop Black)"), hostname, battery, state and plugged-in flash drives.
Rename (placed devices: action; carried laptops: inventory Use) sets the name (printable ASCII, 32 characters, empty =
default); the Game Master action "Rename device" and `ROOT_AE3_API.SetDeviceName` do the same. Names are saved by
persistence.

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
dialog, notifications. Window size Fullscreen / Large / Medium / Small: taskbar button "Size" or F11 cycles it;
the choice is stored per player in `$profile:ROOT_AE3/client.txt` (setting `m_iDesktopSize` is the default, Large).
Open windows keep their content and are fitted into the new size.
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
network transfer speed, games, EWO mode / energy / range / drain / charge / recharge / source range. The same
attributes are on the AE3 System module. Game setting changes are sent to all players.
Attributes: `Configs/Editor/AttributeLists/Edit.conf`, `Scripts/Game/ROOT_AE3/Editor/ROOT_AE3_EditorAttributes.c`.

## 77th JSOC EWO mode

Off by default (setting `m_bEwoMode`, AE3 System module, Game Master attribute "77th JSOC EWO mode"). Packs whose
prefab path contains a name in `m_sEwoPrefabs` (default `Radio_ANPRC77`, the AN/PRC-77 radio backpack) are EWO packs,
worn, carried or placed. Inventory "Use" on a pack in the own inventory or the vicinity opens the EWO panel; it
updates every 3 s and closes when the pack is out of reach, the player dies or EWO mode is turned off:

- Energy pool `m_fEwoEnergyMax` (400; 1 energy = 1 Wh = 1 % of a laptop battery), shown as a bar.
- Wireless network: start / stop, name (default `EWO Net N`) and password (random, 8 characters A-Z without I and O,
  2-9), gateway `77.95.N.1` (N: lowest free number 1-254), range `m_fEwoWifiRange` (100 m), drains `m_fEwoWifiDrain`
  (3 per minute) while broadcasting, stops when the pack is empty.
  External SSH into an EWO network is accepted only from EWO subnets (`77\.95\.\d+\..*`). The network is a router
  (`Prefabs/ROOT_AE3/Devices/ROOT_AE3_EwoRouter.et`) that exists while it broadcasts, follows the pack and is not
  shown on the Map app.
- Laptops in the carrier's inventory with their charge; Charge / Stop (pack carried). Each charging laptop gains `m_fEwoChargeRate`
  (10) percent per minute from the pack; charging ends when the laptop is used, leaves the inventory, is full or the
  pack is empty.
- Plug in / Unplug: the nearest running generator, battery or solar panel within `m_fEwoSourceRange` (10 m) recharges
  the pack at up to `m_fEwoRechargeRate` (5 per minute). The source carries 60 W per energy per minute from what its
  consumers leave, like a charging battery; it unplugs when the source stops, is picked up or is out of range.

Defaults follow Root's Cyber Warfare (`script_macros.hpp`). Server owned; checked every 3 s. A pack is tracked from its
first panel request. Pack state stays while EWO mode is off and is not saved by persistence.

## AE3 System module

`Prefabs/ROOT_AE3/Systems/ROOT_AE3_SystemModule.et` (Game Master Systems, World Editor). Its `m_Settings` replace the
mission header and `Configs/ROOT_AE3/Settings.conf` for the mission; empty = the settings at placement. Devices
follow them where their own value is "Setting default"; values set on a device in the World Editor or by the Game
Master take priority. Game Master: the module's attributes window lists the game settings (see Game Master, plus
EWO), its action panel sets the default root password and the EWO pack prefabs. The first module placed is active;
further modules are inactive and cannot be edited. Deleting the active module activates the next one, else the
mission header or defaults apply. Its settings, including Game Master changes, are saved by persistence. Script API: `ROOT_AE3_API.SetGlobalNumber(key, value)`, `SetGlobalText(key, value)`.

## World Editor

| Component | Attributes |
| --- | --- |
| `ROOT_AE3_ComputerComponent` | interface mode, terminal / desktop access (identity IDs, faction keys, player names), hostname, root login, root password, users (name, password, sudoer), sudoers, install commands / games / crypto and crack, USB ports |
| `ROOT_AE3_SetupComponent` | files and directories, mail addresses, mails, chat handle, web pages, browser history, calendar (with location), wallpaper (0 black, 1-5) |
| `ROOT_AE3_PowerComponent` | role, draw on / standby, output, capacity, recharge, fuel tank, burn rate, mains, external only, on at start, start level, start fuel, buffer output, solar panel tilt |
| `ROOT_AE3_NetworkComponent` | router: SSID, gateway, range, multiplier, accept clients, password, external SSH, allow list, speed; client: static IP, speed, SSH |
| `ROOT_AE3_ConnectionComponent` | power source and router by entity name (cable links) |
| `ROOT_AE3_DeviceComponent` | type, name, hidden (not on the Map app, no player rename) |
| `ROOT_AE3_SystemModuleComponent` | mission settings (AE3 System module) |

## Settings

Defaults in `Configs/ROOT_AE3/Settings.conf`. A mission overrides them with `m_ROOT_AE3_Settings` in its
`SCR_MissionHeader`; an AE3 System module overrides both. The Game Master changes part of them in game (see Game Master).
Groups: Main (debug log, network debug log, any-login test mode, device names), armaOS (root login, sudoers, root password, error sound, transfer speeds
and cap, terminal design, scroll, font size, screen mirror range / interval / viewers / lines / keystroke sync
/ change detection, header texts, boot and shutdown time), Desktop (default interface, file
browsing, theme, map same network, window size, web access), Network (range, multiplier), Power (sync threshold, sync, interval), Games
(enabled, hosting, same network, max tables, turn and abandon timeout, power draw, idle draw,
practice seats, bots), EWO (mode, pack prefabs, energy, range, source range, drain, charge and recharge rate).

## Script API

`ROOT_AE3_API` (authority only): `SetDeviceName`, `SetGlobalNumber`, `SetGlobalText`, `AddUser`, `AddSudoer`, `RemoveSudoer`, `SetRootPassword`, `SetRootLogin`,
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
| EWO packs, System module, device names | local authority | authority | names replicated; EWO panel by RPC |
| Window size, inventory Use | local | none | local (open request by RPC) |
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
viewer's streaming range, Game Master action for multi-page web sites (script API `RegisterSite` only), the pixel canvas API for other mods (Arma 3 `fnc_retro_*`), naming a laptop when it is placed, EWO registration of laptops as Root's Cyber Warfare hacking targets, removing installed games, the Zeus games roster and per-computer
games module, file drag and drop, the terrain map in the Map app (it shows devices on a grid), copying and exporting mail and chat, portable desks and desk chairs, lamp variants (one floodlight), animated GIF, video formats the engine video player does not take, page styles and scripts in the Browser, typing
animations, refuelling generators from fuel trucks or nozzles, the Arma 3 deployment type setting (Reforger
placeable items).
