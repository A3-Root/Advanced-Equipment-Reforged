# Systems

## Power

| Role | Behaviour |
| --- | --- |
| Consumer | Draws on / standby watts from one provider; runs on its internal battery when unsupplied |
| Battery | Charges from the output its provider has left after consumers, supplies its own consumers, switches off on overload |
| Generator | Burns fuel while on, switches off on overload or empty tank; refuel from a carried fuel container |
| Solar panel | Output by sun elevation and panel tilt, buffered in its internal battery |

Plugging: "Plug into power source" connects to the nearest provider within 6 m. Cables drop when either end is
picked up or moved more than 2 m beyond the plugged length.

## Network

Routers take `192.168.N.1/24`, hand out addresses from `.10` (or a static IP inside the subnet), cascade by
cable and accept wireless clients within their range. Static IP, SSH access and router settings come from the
Game Master, World Editor or script API. `wifi scan`, `wifi connect`, the desktop Network app and
the "Plug network cable" action (10 m) join a router. Wireless clients drop out of range or when the router
loses power; cables follow the same stretch rule as power cables. A router without password changes settings
only for sudoers.

## Sessions and access

One player uses a computer at a time. Sessions start within 4 m and end when the player moves away, dies, loses
the character or the computer is picked up. Closing a session ends `su`, SSH and running programs but keeps the
logged-in account for the next player; log out with `exit` or Sign out. Terminal and desktop requests need the open interface and the
access lists. Passwords: 1 s delay per failed attempt, up to 5 s, per player identity.

## Limits

| Item | Limit |
| --- | --- |
| Stored characters per filesystem | 500,000 |
| Characters per file | 65,536 |
| Files, folders and links per filesystem | 5,000 |
| Script lines per run | 50, nesting depth 8 |
| Terminal line | 1,024 characters |
| Log files | cut to the newest 8,000 characters over 16,000 |
| Mails per folder | 100, at most one mail every 2 s per computer |
| Client messages | 60 per second, 65,536 characters each |
| Server output per player | 65,536 characters per second; the message that crosses the limit is still sent |

## Persistence

Device state, links, network settings, installed programs, access lists, flash drives and lid state are saved
by the vanilla persistence system. Game Master "Save" / "Restore" use `$profile:ROOT_AE3/saves/<name>.txt`.
`GetOnDeviceReady` fires again after a restore.
