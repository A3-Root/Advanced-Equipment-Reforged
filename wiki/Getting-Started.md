# Getting Started

## Concepts

- **Device**: any AE3 object (laptop, server, router, flash drive, generator, battery, solar panel, floodlight). Every
  device has a name shown in the inventory and on the Map app.
- **Computer**: a device running armaOS (laptop, portable server). It has a hostname, users, a filesystem and an
  interface mode: terminal only, desktop only or both.
- **Session**: one player using a computer. It starts within 4 m of a placed computer or from the player's own
  inventory and ends when the player walks away, dies or closes it.
- **Power**: devices draw from one provider (generator, battery, solar panel) through a power cable, or from their own
  battery or fuel tank.
- **Network**: routers give addresses to computers connected wirelessly (in range, password) or by network cable.

## First scenario in the Game Master

1. Open the Game Master, Edit mode, and search for "Laptop", "Router" and "Generator" in the entity list.
2. Place a laptop and a router next to each other.
3. Hover the laptop, open the context menu, choose "Advanced Equipment...", then "Add user": name `smith`,
   password `1234`, sudoer on.
4. Choose "Add file" on the laptop: where "Desktop of smith", name `orders.txt`, some text.
5. Switch to the player, walk to the laptop and use "Power on", then "Use desktop". Sign in as `smith` / `1234`.
6. Open the Network app or the tray icon and connect to the router.

## First scenario in the World Editor

1. Place `Prefabs/ROOT_AE3/Devices/Variants/ROOT_AE3_Laptop_Black.et` and a router variant.
2. On the laptop, open `ROOT_AE3_ComputerComponent` and add a user under User accounts.
3. On the laptop, open `ROOT_AE3_SetupComponent` and add files, mails and web pages.
4. Give the router an entity name (`router1`) and enter it in the laptop's `ROOT_AE3_ConnectionComponent`
   `m_sNetwork`: the laptop is cabled to it at mission start.
5. Set `ROOT_AE3_PowerComponent` "Powered on at mission start" on the laptop.

## Next

- [Player Guide](Player-Guide.md)
- [Game Master Guide](Game-Master-Guide.md)
- [World Editor Guide](World-Editor-Guide.md)
