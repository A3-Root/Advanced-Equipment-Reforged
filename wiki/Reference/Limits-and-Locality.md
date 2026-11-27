# Limits and Locality

## Limits

| Item | Limit |
| --- | --- |
| Stored characters per filesystem | 500,000 (flash drives also their capacity) |
| Characters per file | 65,536 |
| Files, folders and links per filesystem | 5,000 |
| Script lines per run | 50, nesting depth 8 |
| Terminal line | 1,024 characters; `grep` and `find` match the first 1,024 characters of each line |
| Command output | 65,536 characters; folder listings 500 entries |
| Log files | cut to the newest 8,000 characters over 16,000 |
| Mails per folder | 100, at most one mail every 2 s per computer |
| Client messages | 60 per second, 65,536 characters each |
| Server output per player | 65,536 characters per second; the message that crosses the limit is still sent |
| Failed passwords | 1 s delay per failure, up to 5 s, per player |

## Sessions

One player uses a computer at a time. Sessions start within 4 m, or from the own inventory, and end when the player
moves away (more than 5 m), dies, loses the character, the lid is closed or the computer is picked up by another
player. A laptop dropped from the operator's inventory keeps the session while the operator stays within 5 m and the
lid is open. A carried computer goes to standby when its session ends. Closing a session ends `su`, SSH and running
programs but keeps the logged-in account unless `m_bLogoutOnSessionEnd` is on.

## Locality

| | SP / listen host | Dedicated server | Client |
| --- | --- | --- | --- |
| Filesystem, users, shell, mail, games | local authority | authority | none (requests by RPC) |
| Power and network simulation, held cables | local authority | authority | replicated state |
| Terminal and desktop UI, sounds | local | none | local |
| Cable lines and coils, flash drive in the port | drawn | not drawn | drawn |
| EWO packs, System module, device names | local authority | authority | names replicated; EWO panel by RPC |
| Window size, inventory Use | local | none | local (open request by RPC) |
| Screen on the model | drawn | not drawn | drawn within range |

Client requests go through `SCR_PlayerController` RPCs; only the session owner receives its output. Screen text goes
by RPC only to players within the screen mirror range, up to the viewer setting per device.
