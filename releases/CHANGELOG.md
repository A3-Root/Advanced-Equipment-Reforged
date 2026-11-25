# Changelog

## 1.0.0

### Added
- Laptop, portable server, router, flash drive, battery pack, solar panel, floodlight and generator prefabs with Blender-built models.
- armaOS terminal: login, sudo / su, redirection, Tab completion, history, 20 designs, 34 programs, scripts.
- Virtual filesystem with owner / everyone permissions, symlinks, trash, USB mounts, locked and media files.
- Desktop with windows, taskbar, start menu, themes, wallpapers and 21 apps.
- Power grid: generators with fuel, batteries, solar output by sun angle, overload, internal batteries.
- Network: routers with DHCP, static IP, cascading, wireless range and password, SSH policy, ping, messages.
- Mail, chat, in-game web pages and sites, browser history, calendar, CCTV feeds.
- Networked games: chess, checkers, uno, ludo, snakes and ladders, blackjack, rock paper scissors, battleship; Snake.
- Screen content on the laptop model for nearby players.
- Game Master context action "Advanced Equipment..." with an action panel, text form and result notifications.
- World Editor components for users, files, mail, web pages, calendar, power and network links.
- Mission header settings, public script API and events, persistence serializers, named save / restore.
- Per-computer terminal / desktop access lists (player names, faction keys) and per-user transfer speed.
- Positional generator and computer running sounds.
- Olive, Sand and Black device variants; desktop window size setting; model LOD1 meshes.
- Arsenal entries (US, USSR, FIA, CIV) for laptops, routers, battery packs, flash drives and portable servers.
- Inventory sizes: flash drive 1x1, router 2x1, laptop and battery pack 2x2, portable server 3x2 (new slot size).
- Portable server as an inventory item.
- Device textures (plastic, metal, rubber, keys, solar cells) from `tools/gen_textures.py`.
- Inventory and arsenal preview cameras; laptop shown closed.
- Workbench plugin `-ae3Items` inventory inspection.
- Optional `crypto` and `crack` programs (caesar, columnar).
- Terminal `wifi` command; password attempts delayed after failures; per-filesystem storage limit, flash drive capacity enforced.
- Game Master device attributes: power, battery, fuel, root login, interface mode, SSH, router range / accept / external SSH.
- Game Master game settings category "Advanced Equipment" (boot / shutdown time, login, desktop, network, games), sent to all players.
- String table `Language/ROOT_AE3_localization.st` for device names (Game Master browser search).
- Game Master browser preview images for every device and variant; vanilla previews for portable generators and floodlight.
- Workbench plugin `-ae3Dump` writes resources (vanilla configs) to a file.

### Changed
- Device variants recolour the body material (router, battery pack, portable server).
- Prefabs list only the material slots their models have.
- Portable server collision uses the vanilla metal game material.
- Solar panel cells sit on top of the frame; back legs end below the panel; LOD1 equals LOD0.
- Desktop, terminal and Game Master panel buttons react to mouse clicks, highlight on hover and press and play the click sound.
- Computer menu and Game Master dialog use input context `ROOT_AE3_ComputerContext`: terminal history on Up / Down only (not W / S); character and editor keys are off while the menu is open.
