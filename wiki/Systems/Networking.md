# Networking

Routers take `192.168.N.1/24` (or a set gateway), hand out addresses from `.10` or a static IP inside the subnet,
cascade by cable and accept wireless clients within their range (default 100 m, `m_fDefaultWirelessRange`, times
`m_fWirelessRangeMultiplier`) with an optional password. Static IP and router settings come from the Game Master, World
Editor or script API.

## Joining a network

- Wireless: `wifi scan`, `wifi connect <ssid|gateway> [password]`, the desktop Network app or the tray quick panel.
  Clients drop out of range or when the router loses power.
- Cable: see [Cables and Flash Drives](Cables-and-Flash-Drives.md). A cable needs physical access and skips the router
  password; it needs a router that accepts connections.
- Game Master: "Connect to network" (router list with distance, wireless or cable, ignore range), "Connect selection to
  router", "Disconnect network".
- World Editor: `ROOT_AE3_ConnectionComponent.m_sNetwork`. Script: `ConnectNetwork`, `DisconnectNetwork`, `SetStaticIp`.

## Remote access

Computers accept SSH logins while `m_bSshEnabled` is on (laptops and the portable server by default); root and sudoers
switch it with `sshd on|off` or the Network app; Game Master attribute "SSH server"; `ROOT_AE3_API.SetSshEnabled`.
`ssh` and Remote Access take an IP address or a hostname. SSH to a computer in standby fails with "Host is in standby."
Routers filter SSH from other subnets (`m_bAllowExternalSsh`, regular expressions in `m_aExternalAllow`). A router
without password changes settings only for sudoers.

## Speeds

Copies are simulated: local `m_fTransferSpeedLocal` (20480 KB/s), USB `m_fTransferSpeedUsb` (2048 KB/s), network
`m_fTransferSpeedNetwork` (512 KB/s), capped by `m_fTransferMaxDuration`; routers and devices set their own speed.

## Map app

Shows devices up to 5 km around. With `m_bDesktopMapSameNetworkOnly` (default on): only the computer, its router,
routers linked to it and devices connected to it; power devices hidden.
