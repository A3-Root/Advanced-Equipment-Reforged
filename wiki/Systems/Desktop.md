# Desktop

Opened with "Use desktop", inventory "Use" on a carried laptop, or the `desktop` command.

## Styles

Each computer has a style:

- **Civilian (Windows)**: icons, tool bars, start menu grid, themes Dark, Light, Olive.
- **Military (Tactical)**: monospace text, outlined controls, bracketed titles, text taskbar and program list,
  themes Green, Amber, Night.

The style comes from the computer (Game Master attribute or action "Desktop style", World Editor
`ROOT_AE3_ComputerComponent.m_iUiStyle`, `ROOT_AE3_API.SetUiStyle`), else the setting `m_iDesktopStyle`. Open sessions
switch at once.

## Shell

- Sign-in screen (Tab: next field; "Use terminal" switches to the terminal).
- Background: black, wallpapers 1-5, an image file or a web address.
- Desktop icons: apps, then the files and folders in `~/Desktop` (file browsing on). Click selects, double click or
  Enter opens, right click for the menu (files: Open, Rename, Delete). Right click on the background: Refresh, New
  folder, New text document, Files, Terminal, Personalize, System info.
- Windows: drag by the title bar; drop on the top edge to maximize, on the left or right edge to fill that half;
  double click the title bar to maximize or restore; resize from the bottom right corner. Ctrl+W closes.
- Taskbar: start button, search, pinned Files / Browser / Email / Terminal, open windows (click to focus or
  minimize, right click to close), window size, tray (flash drive, network, power and battery; click opens the quick
  panel), clock (opens Calendar), show desktop (right edge: minimize or restore all windows).
- Quick panel: network, power and storage state; Network, My Computer, Terminal, Theme, Window size, Settings;
  Standby, Shut down, Sign out.
- Start menu: search box for apps and files (two letters or more; files in the home folder, or the whole computer
  while file browsing is on); Enter opens the first match. User, hostname and power menu.
- Open / Save dialog (Notepad, Photos, Remote Access): places (Home, This computer, Drives, Temp), Up, New folder,
  click selects, double click opens a folder or picks a file, file name field; Enter accepts, Backspace goes up, Esc
  cancels.
- Window size Fullscreen / Large / Medium / Small: taskbar Size button or F11; stored per player in
  `$profile:ROOT_AE3/client.txt` (default `m_iDesktopSize`, Large).
- When a session ends, the client keeps its open windows and unsaved text; they return when the same player opens the
  same computer as the same user.
- Keys reach the focused window while no text field is in write mode.

## Sounds

Start, shut down and standby play a chime for the player using the computer and players within 4 m; flash drives
plugged in and ejected too. Errors, mail and messages play a chime for the player using the computer. Nearby players
hear a switch sound at the device.

## Apps

| App | Use |
| --- | --- |
| My Computer | Volume cards with usage meters; flash drive open, mount, unmount, eject |
| Files | Back / forward / up, address bar, search, navigation pane, large icons or details (sort by header), right-click menus, new folder / text document, cut / copy / paste, rename, delete to Recycle Bin, shortcuts, properties (owner, permissions), unlock. Keys: Enter, Backspace, Delete, F2, F5, arrows, Ctrl+C / X / V |
| Notepad | New, Open..., Save, Save as... |
| Photos | Media files with caption, Web address... (image or video), Paste base64..., Save as..., Set as background; videos: Play, Stop, Restart |
| Terminal | Switches the session to the terminal |
| Browser | Registered web pages, history; http(s) addresses (web access on): pages as text with links, images and videos |
| Email | Inbox / Sent, search, reading pane, compose (From picks an own address), reply, delete, own addresses |
| Messenger | Conversations, search, new conversation, handles, reading as (all or one handle), message bubbles |
| Calendar | Month grid, go to date, Today, day panel with events (time, title, location, details), add, delete |
| Network | Connection status, networks in range with signal and security, connect, disconnect, remote access on / off (root and sudoers) |
| Remote Access | Connect card (user@host, password, recent hosts), remote file browser with back, forward, up, refresh, address bar, preview, Download... and Upload... |
| Ping | Target bar with recent targets, result log |
| Map | Devices up to 5 km around; with `m_bDesktopMapSameNetworkOnly` (default) only the computer, its router, routers linked to it and devices connected to it. Wheel or +/- zooms, drag or arrow keys move, Center |
| CCTV | Every registered camera |
| Games | Lobby, tables and boards, see [Games](Games.md) |
| Snake | Arrow keys or WASD, Enter or Space starts, small / large board, best score |
| Calculator | + - * / %, sign, backspace; keyboard and numpad |
| Settings | Personalize (theme, wallpaper, picture, web address), Terminal (design), Style |
| System Info | Host, power and battery, network, storage |
| Recycle Bin, About | |

## Web access

With `m_bDesktopWebImages` on, each client downloads addresses itself and stores media in
`$profile:ROOT_AE3/media/`. Image addresses go through the web image proxy (`m_sDesktopWebImageProxy`), which returns
the image as base64 text; empty = direct download (addresses that return base64 text only). Images: PNG, JPEG, GIF
(first frame), base64 at most 2 MB. Videos (mp4, webm, ogv, mov, m4v, avi, mkv, bk2, bik): at most 8 MB, played by the
engine video player, which refuses formats it does not take. Pages: text, headings, lists and links.

## File browsing

With `m_bDesktopEnableFileBrowsing` off, My Computer, Files, Notepad, Photos, Recycle Bin and Remote Access are hidden,
the desktop shows no files and opens and saves only files inside the user's home folder.
