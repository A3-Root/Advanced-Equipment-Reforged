# Systems

## Power

| Role | Behaviour |
| --- | --- |
| Consumer | Draws on / standby watts from one provider; runs on its internal battery when unsupplied |
| Battery | Charges from the output its provider has left after consumers, supplies its own consumers, switches off on overload |
| Generator | Burns fuel while on, switches off on overload or empty tank; refuel from a carried fuel container |
| Solar panel | Output by sun elevation and panel tilt, buffered in its internal battery |

Plugging: "Take power cable" on a source, then "Plug power cable into ..." on a device within the source cable length
(`m_fCableLength`). Cables drop when either end is picked up or the devices move farther apart than the cable.

## Network

Routers take `192.168.N.1/24`, hand out addresses from `.10` (or a static IP inside the subnet), cascade by
cable and accept wireless clients within their range. Static IP and router settings come from the
Game Master, World Editor or script API. Remote access (SSH logins, `m_bSshEnabled`, on for laptops and the
portable server): Game Master attribute "SSH server", desktop Network app, `sshd on|off`, `ROOT_AE3_API.SetSshEnabled`.
SSH to a computer in standby fails with "Host is in standby." `wifi scan`, `wifi connect`, the desktop Network app and
the network cable actions (see Cables) join a router. Wireless clients drop out of range or when the router
loses power; cables follow the same stretch rule as power cables. A router without password changes settings
only for sudoers.

## Sessions and access

One player uses a computer at a time. Sessions start within 4 m, or from the own inventory (inventory "Use",
on foot or in a vehicle), and end when the player moves away (more than 5 m), dies, loses the character, the lid is
closed or the computer is picked up by another player. A laptop dropped from the operator's inventory keeps the session
while the operator stays within 5 m and the lid is open. A carried computer goes to standby when its session ends. The client keeps the open desktop windows of the last
session and restores them for the same computer and user. Closing a session ends `su`, SSH and running programs but keeps the
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

## EWO packs

Setting `m_bEwoMode`. A pack matching `m_sEwoPrefabs` (worn, carried or placed) has an energy pool, a wireless
network on a router that exists while it broadcasts, follows the pack and is not on the Map app (`EWO Net N`, random
password, gateway `77.95.N.1` with the lowest free N 1-254, external SSH only from `77.95.*` subnets), charges laptops
in the carrier's inventory and recharges from a running generator, battery or solar panel within
`m_fEwoSourceRange`; the source carries the load from what its consumers leave. Server tick 3 s. Panel: inventory
"Use" on the pack (own inventory or vicinity), updated every tick.

## Cables

Network cable items: 5 m, 15 m, 30 m (`ROOT_AE3_CableComponent.m_fLength`). Two steps (`ROOT_AE3_CableHand`): "Plug in
network cable" on the first device (longest carried cable), then "Plug network cable into ..." on the other end (computer
to router, router to router: the first router joins the second). The server checks length and accept, uses up the item
and stores length and prefab with the link (saved by persistence). Power: "Take power cable" on a source, then "Plug
power cable into ..." on a consumer or battery. The held end is drawn to the right hand and drops beyond the cable
length from the first device. Unplug gives the network cable item back; a link pulled beyond its length unplugs and
drops the item at the device. Clients draw network cables to the router and power links to the
source between the ports of `ROOT_AE3_ConnectionComponent` (procedural tube, sag from the slack, not below the ground).
