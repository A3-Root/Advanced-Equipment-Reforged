# Game Master Guide

## Placing devices

Edit mode lists every device (search "AE3", "Laptop", "Router", "Generator", "Cable"). Placed devices start off; use
"Power on" in the action panel or the attribute "Powered on". Place the AE3 System module (Systems) to change
mission settings in game.

## Action panel

Hover or select one or more devices, open the context menu and choose "Advanced Equipment...". The panel lists the
actions that apply to the selection; an action with fields opens a form with the current values of the first
selected device. Each action reports how many devices it changed and the first error.

| Group | Actions |
| --- | --- |
| Users | Add user, Add sudoer, Remove sudoer, User transfer speed, Root password, Root login policy, Interface access (identity IDs, faction keys or player names for terminal and desktop) |
| Mission settings (System module) | Default root password, EWO pack prefabs, Web image proxy |
| System | Rename device, Hostname, Interface mode, Desktop style (civilian / military), Open interface as root, Crash, Save computer, Restore computer |
| Files and intel | Add file, Add directory, Add image, Add email, Add mail address, Chat handle, Add web page, Add browser history, Add calendar event, Install games |
| Power | Power on, Power off, Standby, Battery level, Fuel, Power source, Power cable length, Connect power (selection), Disconnect power |
| Network | Router name and password, Connect to network, Static IP, Transfer speed, Connect selection to router, Disconnect network |

### Add file

| Field | Meaning |
| --- | --- |
| Where to put the file | Desktop or home folder of a user, the top folder `/`, or another folder |
| Folder | Shown for "Other folder"; created when missing |
| File name, Text in the file | Name with extension (`orders.txt`) and content |
| Who can open it | Everyone, only its owner (and root), or only root |
| Owner | User account that owns the file: root, an account of the computer, or "New account" (name and password; the account is created when missing) |
| Protection | None, a password (players run `unlock FILE PASSWORD`), a cipher, or both |
| Cipher | One of 15 ciphers; the next field asks for its key in plain words (shift, key word, variant, rails, alphabet, number base) |
| Shown file size | Size `ls -l` and Files show instead of the real size |

### Power source and cables

"Power source" lists every power source within 500 m with its distance and cable length; choosing one links the
device without a cable item, at any distance. "None (unplug)" removes the link. "Connect power (selection)" links the
selected devices to the selected source. "Power cable length" sets the cable players take from a source.

### Desktop style

"Desktop style (civilian / military)" sets the look of the selected computers: Mission setting, Civilian (Windows)
or Military (Tactical). Open sessions switch at once.

### Open interface as root

Opens the desktop as root for the Game Master without range or access checks. The previous login is restored when it
closes.

### Save and restore

"Save computer" writes the computer state to `$profile:ROOT_AE3/saves/<name>.txt` on the server; "Restore computer"
loads it (empty name lists the saves).

## Device attributes

Attributes window, category "Advanced Equipment":

| Attribute | Devices |
| --- | --- |
| Powered on | Power devices |
| Battery level | Devices with a battery |
| Fuel, Fuel use (L/h, 0 = no fuel used) | Generators, floodlight |
| Root login, Interface, Desktop style, SSH server | Computers |
| Wireless range, Transfer speed, Accept connections, External SSH | Routers (transfer speed also computers) |

## AE3 System module

The module's attributes window holds the mission settings: boot and shutdown time, root login, sudoers act as root,
any login, log out on session end, default interface, desktop theme, desktop style, window size, file browsing,
screen on models, router range and multiplier, network transfer speed, games and EWO mode. Changes reach all
players and are saved by persistence. Only the first placed module is active.
