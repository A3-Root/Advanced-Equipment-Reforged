# Settings

`ROOT_AER_Settings`. Defaults in `Configs/ROOT_AER/Settings.conf`; a scenario overrides them with
`m_ROOT_AER_Settings` in its `SCR_MissionHeader`; an AER System module (`m_Settings`) overrides both. Devices follow
the settings where their own value is "Setting default". The Game Master changes the settings with a key in the
System module's attributes; script: `ROOT_AER_API.SetGlobalNumber(key, value)` / `SetGlobalText(key, value)`.
Changes are sent to all players.

## Main

| Setting | Key | Default | Meaning |
| --- | --- | --- | --- |
| `m_bDebugMode` | | 0 | Debug log output |
| `m_bDebugAnyLogin` | `debug_any_login` | 0 | Testing only: an unknown user name logs in with any password and the account is created |
| `m_bNetworkDebug` | | 0 | Network debug log output |
| `m_iDeviceNameFallback` | | 0 | Device name when none is set: generic type name or prefab display name |

## armaOS

| Setting | Key | Default | Meaning |
| --- | --- | --- | --- |
| `m_bAllowRootLogin` | `allow_root_login` | 0 | Root may log in at the terminal; otherwise a user plus sudo / su |
| `m_bCliElevateSudoers` | `cli_elevate` | 1 | Sudoers act as root on the filesystem |
| `m_bLogoutOnSessionEnd` | `logout_on_end` | 0 | Log the account out when a session ends |
| `m_sDefaultRootPassword` | `root_password` (text) | toor | Root password of computers without their own |
| `m_bEnableErrorSound` | `error_sound` | 1 | Error sound when a command fails |
| `m_fTransferSpeedLocal` / `Usb` / `Network` | `transfer_local` / `transfer_usb` / `transfer_network` | 20480 / 2048 / 512 | Simulated copy speed in KB/s, 0 = instant |
| `m_fTransferMaxDuration` | | 0 | Longest simulated transfer in s, 0 = no cap |
| `m_iTerminalDesign` | `terminal_design` | 0 | Default terminal design 0-19 |
| `m_iTerminalScrollSpeed` | | 1 | Lines per mouse wheel step |
| `m_fTerminalDefaultSize` | | 0.75 | Terminal font scale |
| `m_bUiOnTexture` | `ui_on_texture` | 1 | Screen content on the device model for nearby players |
| `m_fUiPlayerRange` | `ui_range` | 2 | Range in m for the screen content |
| `m_fUiOnTexUpdateInterval` | | 1 | Screen mirror interval in s |
| `m_iUiMaxConcurrentViewers` | | 3 | Players receiving one screen, 0 = none, -1 = unlimited |
| `m_iUiMaxTransmitLines` | | 22 | Lines mirrored to the screen |
| `m_fUiKeystrokeSyncInterval` | | 0.3 | Interval in s for mirroring the typed line |
| `m_bUiEnableChangeDetection` | | 1 | Mirror only changed screens |
| `m_sTerminalDialogTitle`, `m_sTerminalBiosVersion`, `m_sTerminalCopyright`, `m_sTerminalBootMessage`, `m_sTerminalTipMessage`, `m_bTerminalShowAsciiArt`, `m_sTerminalTagline` | | FUBAR(TM) texts | Terminal title and boot header |
| `m_fStartupTime` / `m_fShutdownTime` | `startup_time` / `shutdown_time` | 15 / 15 | Boot and shutdown duration in s |

## Desktop

| Setting | Key | Default | Meaning |
| --- | --- | --- | --- |
| `m_iDesktopDefaultMode` | `default_mode` | 2 | Interface of computers without their own: 0 terminal, 1 desktop, 2 both |
| `m_bDesktopEnableFileBrowsing` | `file_browsing` | 1 | File browsing apps and desktop files |
| `m_iDesktopDefaultTheme` | `default_theme` | 0 | Theme 0-2 |
| `m_iDesktopStyle` | `desktop_style` | 0 | 0 civilian (Windows), 1 military (Tactical) for computers without their own |
| `m_iDesktopSize` | `desktop_size` | 1 | Window size 0 fullscreen, 1 large, 2 medium, 3 small |
| `m_bDesktopWebImages` | `web_images` | 1 | Web addresses in Browser, Photos and wallpapers |
| `m_sDesktopWebImageProxy` | `web_image_proxy` (text) | wsrv.nl address | Address returning an image as base64, `%1` = image address; empty = direct |
| `m_bDesktopMapSameNetworkOnly` | `map_same_network` | 1 | Map app shows only the own network |

## Network and power

| Setting | Key | Default | Meaning |
| --- | --- | --- | --- |
| `m_fDefaultWirelessRange` | `range_default` | 100 | Router range in m |
| `m_fWirelessRangeMultiplier` | `range_multiplier` | 1 | Multiplier on every range |
| `m_fPowerChangeThreshold` | | 0.01 | Battery change that is replicated |
| `m_bPowerEnableStateSync` | | 1 | Replicate battery levels |
| `m_fPowerUpdateInterval` | `power_interval` | 5 | Power simulation interval in s |

## Games

| Setting | Key | Default | Meaning |
| --- | --- | --- | --- |
| `m_bGamesEnabled` | `games_enabled` | 1 | Games enabled |
| `m_bGamesAllowHosting` | `games_hosting` | 1 | Players may open tables |
| `m_bGamesRequireSameNetwork` | `games_same_network` | 0 | Joining needs a shared network |
| `m_iGamesMaxTables` | | 16 | Open tables |
| `m_fGamesTurnTimeout` / `m_fGamesAbandonTimeout` | | 120 / 600 | Turn skip and table close in s |
| `m_fGamesPowerDraw` / `m_fGamesIdleDraw` | | 15 / 2 | Extra draw in W |
| `m_bGamesDebugSeats` | `games_practice` | 1 | Practice seats |
| `m_bGamesBots` | `games_bots` | 1 | Bot players |

## EWO

| Setting | Key | Default | Meaning |
| --- | --- | --- | --- |
| `m_bEwoMode` | `ewo_mode` | 0 | 77th JSOC EWO mode |
| `m_sEwoPrefabs` | `ewo_prefabs` (text) | Radio_ANPRC77 | Pack prefab names |
| `m_fEwoEnergyMax` | `ewo_energy` | 400 | Pack energy |
| `m_fEwoWifiRange` | `ewo_range` | 100 | Network range in m |
| `m_fEwoSourceRange` | `ewo_source_range` | 10 | Range to the recharging source in m |
| `m_fEwoWifiDrain` / `m_fEwoChargeRate` / `m_fEwoRechargeRate` | `ewo_drain` / `ewo_charge` / `ewo_recharge` | 3 / 10 / 5 | Energy per minute |
