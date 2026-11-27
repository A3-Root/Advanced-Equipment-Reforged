# Cables and Flash Drives

## Cables

Players plug cables in two steps (`ROOT_AE3_CableHand`, server owned). The first action puts one end in a device and
the other in the player's hand; the second action on the other device plugs it in. The second step fails while the
devices are farther apart than the cable and the cable stays in the hand. The held end drops when the player walks
farther than the cable (+1 m) from the first device, dies, leaves, the first device is picked up or destroyed, or the
network cable item is no longer carried.

| | Network cable | Power cable |
| --- | --- | --- |
| Item | 5 m, 15 m, 30 m (`ROOT_AE3_CableComponent.m_fLength`), used up when plugged | none; part of the source |
| First step | "Plug in network cable" on a computer or router (several lengths carried: a form asks which) | "Take power cable" on a generator, battery or solar panel |
| Second step | "Plug network cable into ..." on the other end: computer to router, or router to router (the first joins the second) | "Plug power cable into ..." on a consumer or battery without a source |
| Length | the item | the source (`m_fCableLength`): airfield generators 25 m, military 20 m, portable 15 m, solar panel 10 m, battery 5 m |
| Unplug | "Unplug network cable" on the device; the item goes back to the inventory | "Unplug power cable" on the device |
| Cancel | "Unplug held network cable" on the first device | "Put power cable back" on the source |

A link drops when the devices move farther apart than the cable, or one of them is picked up. A network cable item
then goes into the inventory of the player who picked the device up, or lies at the device. Links from the Game
Master, World Editor and script need no item; their length is the longer of the source cable and the distance + 2 m.
Cable length and item are saved by persistence.

### Drawing

Every client draws plugged cables between the ports of `ROOT_AE3_ConnectionComponent` (network cables black 7 mm,
power cables orange 11 mm) from segment models, sagging with the slack and kept above the terrain and the floor under
the middle. A cable with more than 1.5 m of spare length runs to a coil on the floor halfway. The held end is drawn to
the right hand of the local player.

## Flash drives

- "Insert flash drive" plugs the first drive from the inventory into a free USB port; the drive prop shows in the port
  and stays attached while the laptop is carried. The drive is mounted at `/mnt/usbN`.
- "Eject flash drive" (highest occupied port) or My Computer "Eject" (any port) put the drive back into the inventory
  or onto the ground with its files.
- `lsusb` lists ports, `mount usbN` / `umount usbN` mount and unmount. Copies to a drive run at `m_fTransferSpeedUsb`.
- A drive holds its capacity (`m_iCapacity`, 8192 MB) and at most 500,000 characters.
- Files are added in the World Editor (`ROOT_AE3_UsbDriveComponent`), by the Game Master (add file / directory / image)
  and by script.
