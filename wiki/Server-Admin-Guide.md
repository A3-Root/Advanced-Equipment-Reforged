# Server Admin Guide

## Running

Add the addon to the server `config.json` mods list. The server runs the filesystem, accounts, shell, power and
network simulation; clients draw the terminal, desktop, screens, cables and sounds. No server file settings are needed.

## Settings

Defaults are in `Configs/ROOT_AE3/Settings.conf`. A scenario overrides them with `m_ROOT_AE3_Settings` in its
`SCR_MissionHeader`; an AE3 System module overrides both and the Game Master changes them in game. Setting changes are
sent to all players. See [Settings](Reference/Settings.md).

## Persistence

The vanilla persistence system saves every device (`Configs/Systems/Persistence/Configuration/ROOT_AE3_Devices.conf`):
filesystem, users, history, preferences, calendar, mail addresses, own web pages, plugged flash drives and their
mounts, lid, access lists, power state, battery, fuel, flash drive content, power and network links with cable length
and item, router and client network settings, and the System module settings. Web pages for every computer come from
the World Editor again; ones added by the Game Master or script for every computer are not saved. Game Master
"Save computer" writes `$profile:ROOT_AE3/saves/<name>.txt` on the server.

## Client files

Each client stores its desktop window size in `$profile:ROOT_AE3/client.txt` and downloaded web media in
`$profile:ROOT_AE3/media/`.

## Web access

With `m_bDesktopWebImages` on, the Browser, Photos and wallpapers load http(s) addresses on each client. Image
addresses go through `m_sDesktopWebImageProxy` (default `https://wsrv.nl/?output=png&w=1024&encoding=base64&url=%1`),
which receives the image address. Turn web access off to keep clients from contacting outside addresses.

## Logs

Warnings go to the log with the prefix `[AE3]`; debug lines too while `m_bDebugMode` is on, network events with
`[AE3 net]` while `m_bNetworkDebug` is on. Failed logins and unlocks go to `/var/log/auth.log` on the computer.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| "No power." | The device needs a running source plugged in, its own battery or fuel |
| Network cable refused | Devices farther apart than the cable, router not accepting connections, device already cabled |
| Desktop does not open | Interface mode and the desktop access list of the computer |
| Root login refused | `m_bAllowRootLogin` off: log in as a user and use `sudo` / `su` |
