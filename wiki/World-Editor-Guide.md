# World Editor Guide

## Devices

Place the variants in `Prefabs/ROOT_AE3/Devices/Variants/` (Black, Olive, Sand) and the prefabs in
`Prefabs/ROOT_AE3/Power/`. The base prefabs in `Prefabs/ROOT_AE3/Devices/` are parents only.
See [Devices](Systems/Devices.md) for the list.

## Accounts and access

`ROOT_AE3_ComputerComponent`:

- User accounts (name, password, sudoer), additional sudoers, root password, root login.
- Interface: setting default, terminal only, desktop only or both. Desktop style: setting, Civilian (Windows) or
  Military (Tactical).
- Terminal and desktop access lists: identity IDs, faction keys or player names; empty = everyone.
- Install commands, games, crypto and crack.

## Files and intel

`ROOT_AE3_SetupComponent`: files and directories (path, content, owner, permissions, encryption and key, password,
shown size), mail addresses, mails, chat handle, web pages, browser history, calendar events, wallpaper.
Flash drives take files in `ROOT_AE3_UsbDriveComponent`.

Permissions are six characters: owner read, write, execute, then everyone read, write, execute, `-` for unset
(`rw-r--`). See [Files and Intel](Systems/Files-and-Intel.md) and [Encryption](Systems/Encryption.md).

## Links at mission start

Give power sources and routers an entity name and enter it in `ROOT_AE3_ConnectionComponent` of each device:
`m_sPowerSource` (power cable to the source) and `m_sNetwork` (network cable to the router, or for a router its parent
router). Links made here need no cable item; their length is the longer of the source cable and the distance + 2 m.

## Power and network

- `ROOT_AE3_PowerComponent`: role, draw, battery, fuel tank and use, mains, on at start, start levels, power cable length.
  A consumer with a fuel tank runs on it without a source.
- `ROOT_AE3_NetworkComponent`: router SSID, gateway, range, password, accept clients, external SSH rules, speed;
  client static IP, speed, SSH.

## Mission settings

Set `m_ROOT_AE3_Settings` in the mission header, or place `Prefabs/ROOT_AE3/Systems/ROOT_AE3_SystemModule.et` and set
its `m_Settings`. The module wins over the header; a value set on a device wins over both. See
[Settings](Reference/Settings.md).

## Scripts

Use `ROOT_AE3_API.GetOnDeviceReady()` to add content from a script once a computer is set up. See
[Script API](Reference/API.md).
