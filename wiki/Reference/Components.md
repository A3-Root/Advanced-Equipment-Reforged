# Components

World Editor attributes of the AE3 components.

| Component | Attributes |
| --- | --- |
| `ROOT_AE3_DeviceComponent` | type, name, hidden (not on the Map app, no player rename) |
| `ROOT_AE3_ComputerComponent` | interface mode, desktop style, hostname, root login, root password, users (name, password, sudoer), sudoers, terminal / desktop access (identity IDs, faction keys, player names), install commands / games / crypto and crack, USB ports and positions |
| `ROOT_AE3_SetupComponent` | files and directories, mail addresses, mails, chat handle, web pages, browser history, calendar events (with location), wallpaper (0 black, 1-5) |
| `ROOT_AE3_PowerComponent` | role, draw on / standby, output, battery capacity, recharge, fuel tank, fuel use, solar tilt, buffer output, mains, external only, on at start, start level, start fuel, power cable length |
| `ROOT_AE3_NetworkComponent` | router: SSID, gateway, range, multiplier, accept clients, password, external SSH, allow list, speed; client: static IP, speed, SSH |
| `ROOT_AE3_ConnectionComponent` | power source and router by entity name (links at mission start), network and power port position and direction |
| `ROOT_AE3_UsbDriveComponent` | label, capacity in MB, port prop, files and directories |
| `ROOT_AE3_CableComponent` | network cable length |
| `ROOT_AE3_LidComponent` | open and closed models, material remaps, open at start |
| `ROOT_AE3_LampComponent` | lights `SCR_BaseInteractiveLightComponent` while the power component is on |
| `ROOT_AE3_DeviceSoundComponent` | looping sound event while on |
| `ROOT_AE3_SystemModuleComponent` | mission settings (AE3 System module) |

## File entries

`ROOT_AE3_FileEntry` (setup component, flash drives): path, directory, content, owner, permissions, encryption
(`ROOT_AE3_Encryption` id), key (empty = default), password, shown size.

## User actions

| Class | Notes |
| --- | --- |
| `ROOT_AE3_UseComputerAction` | Use terminal / desktop |
| `ROOT_AE3_PowerAction`, `ROOT_AE3_StandbyAction`, `ROOT_AE3_LidAction` | Power, standby, lid |
| `ROOT_AE3_ConnectPowerAction` | `m_bSource` on: "Take power cable" (sources); off: plug / unplug on devices |
| `ROOT_AE3_ConnectNetworkAction` | Network cable steps |
| `ROOT_AE3_UsbAction`, `ROOT_AE3_RefuelAction`, `ROOT_AE3_StatusAction`, `ROOT_AE3_RenameAction` | Flash drives, refuel, status, rename |

Laptop, router, battery pack and portable server actions use the inherited `default` context (with "Pick up");
solar panel, floodlight and generators use the context `ae3`. Port positions and directions are model-relative
(+Z = back of the AE3 models).
