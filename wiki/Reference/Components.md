# Components

World Editor attributes of the AER components.

| Component | Attributes |
| --- | --- |
| `ROOT_AER_DeviceComponent` | type, name, hidden (not on the Map app, no player rename) |
| `ROOT_AER_ComputerComponent` | interface mode, desktop style, hostname, root login, root password, users (name, password, sudoer), sudoers, terminal / desktop access (identity IDs, faction keys, player names), install commands / games / crypto and crack, USB ports and positions |
| `ROOT_AER_SetupComponent` | files and directories, mail addresses, mails, chat handle, web pages, browser history, calendar events (with location), wallpaper (0 black, 1-5) |
| `ROOT_AER_PowerComponent` | role, draw on / standby, output, battery capacity, recharge, fuel tank, fuel use, solar tilt, buffer output, mains, external only, on at start, start level, start fuel, power cable length |
| `ROOT_AER_NetworkComponent` | router: SSID, gateway, range, multiplier, accept clients, password, external SSH, allow list, speed; client: static IP, speed, SSH |
| `ROOT_AER_ConnectionComponent` | power source and router by entity name (links at mission start), network and power port position and direction |
| `ROOT_AER_UsbDriveComponent` | label, capacity in MB, port prop, files and directories |
| `ROOT_AER_CableComponent` | network cable length |
| `ROOT_AER_LidComponent` | open and closed models, material remaps, open at start |
| `ROOT_AER_LampComponent` | lights `SCR_BaseInteractiveLightComponent` while the power component is on |
| `ROOT_AER_DeviceSoundComponent` | looping sound event while on |
| `ROOT_AER_SystemModuleComponent` | mission settings (AER System module) |

## File entries

`ROOT_AER_FileEntry` (setup component, flash drives): path, directory, content, owner, permissions, encryption
(`ROOT_AER_Encryption` id), key (empty = default), password, shown size.

## User actions

| Class | Notes |
| --- | --- |
| `ROOT_AER_UseComputerAction` | Use terminal / desktop |
| `ROOT_AER_PowerAction`, `ROOT_AER_StandbyAction`, `ROOT_AER_LidAction` | Power, standby, lid |
| `ROOT_AER_ConnectPowerAction` | `m_bSource` on: "Take power cable" (sources); off: plug / unplug on devices |
| `ROOT_AER_ConnectNetworkAction` | Network cable steps |
| `ROOT_AER_UsbAction`, `ROOT_AER_RefuelAction`, `ROOT_AER_StatusAction`, `ROOT_AER_RenameAction` | Flash drives, refuel, status, rename |

Laptop, router, battery pack and portable server actions use the inherited `default` context (with "Pick up");
solar panel, floodlight and generators use the context `aer`. Port positions and directions are model-relative
(+Z = back of the AER models).
