# Script API

`ROOT_AER_API` (`Scripts/Game/ROOT_AER/API/ROOT_AER_API.c`). Calls change state on the authority (server, or the
host in singleplayer / listen host) and return false elsewhere. Devices are entities. `bool` calls return false when
the device lacks the component or the call fails; `GetLastError()` holds the reason of the last failed `AddFile`.

Computers accept calls once set up: use `GetOnDeviceReady()` (`device, restored`) for devices created at mission
start. It fires again after a persistence or Game Master restore with `restored` true, so add content only when
`restored` is false. Custom commands added with a device are not saved: add them on every call.

## Devices and settings

| Call | Effect |
| --- | --- |
| `SetDeviceName(device, name)` | Name shown in the inventory and on the Map app |
| `SetHostname(device, hostname)` | Computer hostname |
| `SetInterfaceMode(device, mode)` | -1 setting, 0 terminal, 1 desktop, 2 both |
| `SetUiStyle(device, style)` | -1 setting, 0 civilian (Windows), 1 military (Tactical) |
| `SetInterfaceAccess(device, mode, entries)` | Access list (mode 0 terminal, 1 desktop): identity IDs, faction keys, player names |
| `SetGlobalNumber(key, value)`, `SetGlobalText(key, value)` | Mission setting by key (System module keys) |
| `SaveComputer(device, name)`, `RestoreComputer(device, name)` | `$profile:ROOT_AER/saves/<name>.txt` |

## Users

| Call | Effect |
| --- | --- |
| `AddUser(device, user, password, sudoer = false)` | Creates or updates an account with a home folder |
| `AddSudoer(device, user)`, `RemoveSudoer(device, user)` | `/etc/sudoers` |
| `SetRootPassword(device, password)` | Root password |
| `SetRootLogin(device, policy)` | 0 setting, 1 allow, 2 deny |

## Files and intel

| Call | Effect |
| --- | --- |
| `AddFile(device, path, content, owner = "root", perms = "rw-r--", encryption = 0, key = "", password = "", size = -1)` | File on a computer or flash drive; `encryption` is a `ROOT_AER_Encryption` id (see [Encryption](../Systems/Encryption.md)), empty key = default key |
| `AddDirectory(device, path, owner = "root", perms = "rwxr-x")` | Folder |
| `ReadFile(device, path, out content)`, `RemovePath(device, path)` | Read or delete |
| `AddMedia(device, path, source, caption = "")` | Image or video file (texture resource, http(s) address or base64) |
| `AddMailAddress(device, address)`, `AddEmail(device, from, subject, body, to = "", date = "")` | Mail |
| `SetChatHandle(device, handle)` | Chat handle |
| `RegisterWebpage(url, title, markdown, device = null)` | Web page for one computer or every computer |
| `RegisterSite(domain, paths, titles, contents)` | Site with several pages |
| `AddBrowserHistory(device, url, date = "")` | Browser history |
| `AddCalendarEvent(device, date, time, title, text, location = "")`, `RemoveCalendarEvent(device, date, title = "")` | Calendar |
| `RegisterCamera(camera, name)` | CCTV camera |
| `AddGames(device, names = null)`, `HostGameWithBots(device, game, bots = 1)` | Install games, open a table with bots |

## Power and network

| Call | Effect |
| --- | --- |
| `SetPowerState(device, state)`, `GetPowerState(device)` | 0 off, 1 on, 2 standby |
| `CrashDevice(device)` | Blue screen |
| `SetBatteryLevel(device, level)`, `GetBatteryLevel(device)` | 0-1 |
| `SetFuel(device, liters)`, `GetFuel(device)`, `SetFuelUse(device, lph)` | Generators and devices with a tank; 0 L/h = no fuel used |
| `SetMainsPowered(device, mains)` | Permanent power |
| `ConnectPower(consumer, source)`, `DisconnectPower(consumer)` | Power link without a cable item |
| `SetPowerCableLength(source, meters)` | Power cable length players take from a source |
| `ConnectNetwork(device, router)`, `DisconnectNetwork(device)` | Network cable without an item |
| `SetStaticIp(device, ip)`, `GetAddress(device)` | Address |
| `SetSshEnabled(device, enabled)` | Remote access |
| `SetTransferSpeed(device, kbps)` | Transfer speed of a device or router, 0 = setting |

## Programs and desktop

| Call | Effect |
| --- | --- |
| `AddCustomCommand(device, name, description, manual, callback, path = "")` | Program for one computer, or every computer with `null` |
| `InstallCommand(device, name)` | Installs a registered program (also `crypto`, `crack`) |
| `RegisterGuiCommand(cmd, handler)` | Server handler for desktop requests of other mods |
| `RegisterExtApp(appType)` | Client: desktop app class of another mod |

## Events

| Invoker | Arguments |
| --- | --- |
| `GetOnLogin()` | computer, user |
| `GetOnCommand()` | computer, user, program, arguments |
| `GetOnFileChanged()` | computer, path (every change on the computer's own filesystem except log, mail and chat writes) |
| `GetOnDeviceReady()` | device, restored |

## Hooks

`AddAccessCondition` / `RemoveAccessCondition`, `AddConnectCondition` / `RemoveConnectCondition`,
`AddRangeModifier` / `RemoveRangeModifier`: see [Extending AER](../Developer/Extending.md).

## Example

```c
IEntity laptop = GetGame().GetWorld().FindEntityByName("laptop1");
ROOT_AER_API.AddUser(laptop, "smith", "1234", true);
ROOT_AER_API.AddFile(laptop, "/home/smith/Desktop/orders.txt", "Move at dawn.", "smith", "rw----");
ROOT_AER_API.AddFile(laptop, "/home/smith/codes.txt", "ALPHA", "smith", "rw----", ROOT_AER_Encryption.VIGENERE, "LEMON", "secret");
ROOT_AER_API.RegisterWebpage("intel.local", "Intel", "# Intel\n\n[Home](rootnet.home)");
ROOT_AER_API.SetUiStyle(laptop, 1);
```
