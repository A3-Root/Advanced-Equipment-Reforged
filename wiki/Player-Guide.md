# Player Guide

## Actions on devices

Look at a device and open its action menu. The server checks that the player is alive and within 5 m.

| Action | Devices | Effect |
| --- | --- | --- |
| Use terminal / Use desktop | Computers | Opens the interface (placed computers; within 4 m) |
| Power on / Shut down, Power off | Computers, routers, lamps, solar panels | Boots computers (15 s) or shuts them down; switches other devices |
| Standby / Wake up | Computers | Sleeps without closing programs |
| Open / Close lid | Laptops | A closed lid ends the session and blocks power on |
| Start / Stop generator | Generators | Needs fuel |
| Refuel | Generators, floodlight | Drains a carried fuel container such as a jerry can |
| Take power cable / Put power cable back / Plug power cable into ... / Unplug power cable | Power sources, devices | See [Cables](#cables) |
| Plug in network cable / Plug network cable into ... / Unplug held network cable / Unplug network cable | Computers, routers | See [Cables](#cables) |
| Insert / Eject flash drive | Computers with USB ports | Plugs the first drive from the inventory, ejects the highest occupied port |
| Check status | All | Name, state, battery, fuel, load |
| Rename | All placed devices | Name shown in the inventory and on the Map app (32 characters) |

Failed actions show the reason as a hint.

## Carrying a laptop

Laptops, routers, battery packs, flash drives, cables and the portable server are picked up like other items.

- Inventory "Use" on a laptop in the own inventory (on foot or in a vehicle) wakes or boots it and opens the desktop
  (the terminal for terminal-only computers). The session lasts while the laptop stays in that inventory.
- Picking up a laptop keeps its flash drives plugged in, unplugs power and network cables (a network cable item goes
  into the inventory or onto the ground), ends other players' sessions and puts it into standby.
- Carried laptops use wireless networks in range of the carrier.

## Cables

Cables plug in two steps. The first action puts one end in a device and the other end in the player's hand (drawn to
the right hand); the second action on the other device plugs it in. The second step fails while the devices are
farther apart than the cable. The held end drops when the player walks farther than the cable from the first device,
dies, leaves or no longer carries the network cable.

- **Network cable**: carry a network cable item (5 m, 15 m or 30 m). "Plug in network cable" on a computer or router
  (with several lengths a form asks which cable), then "Plug network cable into ..." on the other end: computer to
  router, or router to router (the first router joins the second). The item is used up; "Unplug network cable"
  returns it.
- **Power cable**: "Take power cable" on a generator, battery or solar panel (airfield generators 25 m, military
  generator 20 m, portable generators 15 m, solar panel 10 m, battery 5 m), then "Plug power cable into ..." on a
  device that uses power. "Unplug power cable" on the device removes it.

Plugged cables are drawn between the devices. Spare length lies in a coil on the floor halfway. Moving the devices
farther apart than the cable unplugs it.

## Flash drives

"Insert flash drive" plugs the first drive from the inventory into a free port; the drive shows in the port and is
mounted at `/mnt/usb0` or `/mnt/usb1`. Eject it with the action, the My Computer app or `umount` and the action.
Files on a drive travel with it.

## Terminal

Log in with a user name and password (`LOGIN>` / `PASSWORD>`). The prompt is `user@host:/path/>`; typed text and the
cursor follow it. Enter runs the line, Tab completes, Up / Down recall earlier lines, the mouse wheel scrolls. `help`
lists the programs, `man <program>` explains one. See [Terminal](Systems/Terminal.md).

## Desktop

Sign in, then use the start menu, desktop icons or the taskbar. The search box finds apps and files. The tray opens a
quick panel with network, power, storage, theme, window size and power buttons. F11 or the Size button changes the
window size. See [Desktop](Systems/Desktop.md).

## Intel

Encrypted files decrypt with `crypto` (key known) or `crack` (key unknown). Locked files open with
`unlock FILE PASSWORD`. See [Encryption](Systems/Encryption.md).
