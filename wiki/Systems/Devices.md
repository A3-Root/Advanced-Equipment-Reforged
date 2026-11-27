# Devices

| Prefab | Role |
| --- | --- |
| `Devices/Variants/ROOT_AE3_Laptop_<Black/Olive/Sand>.et` | Computer, inventory item (2x2, 2.2 kg), 2 USB ports, screen on the model, lid, internal battery (0.1 kWh, 10 W on, 0.1 W standby) |
| `Devices/Variants/ROOT_AE3_PortableServer_<colour>.et` | Headless computer on mains (150 W), remote access (SSH) on, inventory item (3x2, 25 kg) |
| `Devices/Variants/ROOT_AE3_Router_<colour>.et` | Router, inventory item (2x1, 0.8 kg), 100 m wireless range, internal battery |
| `Devices/ROOT_AE3_UsbStick.et`, `Variants/ROOT_AE3_UsbStick_<Blue/Yellow>.et` | Flash drive, 8 GB filesystem, inventory item (1x1, 0.02 kg) |
| `Devices/ROOT_AE3_NetworkCable.et`, `Variants/ROOT_AE3_NetworkCable_<5m/30m>.et` | Network cable 15 m (0.1 kg), 5 m (0.05 kg), 30 m (0.2 kg), inventory item (1x1) |
| `Devices/Variants/ROOT_AE3_BatteryPack_<colour>.et` | Battery 0.6 kWh, recharges at 300 W, power cable 5 m, inventory item (2x2, 12 kg) |
| `Devices/Variants/ROOT_AE3_SolarPanel_<colour>.et` | 150 W peak by sun elevation and panel tilt, 0.4 kWh buffer, power cable 10 m |
| `Power/ROOT_AE3_Generator_Portable_<US/USSR>.et` | Generator 5 kW, 5 L tank, 1.5 L/h, power cable 15 m |
| `Power/ROOT_AE3_Generator_Military_USSR.et` | Generator, power cable 20 m |
| `Power/ROOT_AE3_Generator_Airfield_<US/USSR>.et` | Generators up to 400 kW, power cable 25 m |
| `Power/ROOT_AE3_Floodlight.et` | Lamp 150 W; built-in generator 10 L, 0.5 L/h, or an external power cable |
| `Systems/ROOT_AE3_SystemModule.et` | AE3 System module (mission settings) |

Paths are under `Prefabs/ROOT_AE3/`. Colours: Black (orange accents), Olive (coyote accents), Sand (ranger green
accents). Computers and routers have a green power and a blue network LED. Generators and computers play their running
sound while on.

Inventory items fit uniform, vest and backpack storage; the 3x2 slot is added by
`Scripts/Game/ROOT_AE3/Inventory/ROOT_AE3_SlotSize.c`. The inventory shows a device's name
("<name> (Laptop Black)"), hostname, battery, state and plugged-in flash drives.

## Names

Every device has a name: the World Editor name, a name set by Rename (players, placed devices), the Game Master
action "Rename device" or `ROOT_AE3_API.SetDeviceName`, else a generic or prefab name
(`m_iDeviceNameFallback`). Names are printable ASCII, 32 characters, saved by persistence. Hidden devices
(`ROOT_AE3_DeviceComponent.m_bHidden`) are not shown on the Map app and cannot be renamed by players.

## Damage

Destroyed devices switch off and offer no actions.
