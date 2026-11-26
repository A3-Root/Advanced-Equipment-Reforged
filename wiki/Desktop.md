# Desktop

Opened with "Use desktop" or the `desktop` command. Sign-in screen (Tab: next field), black background,
wallpapers 1-5, an image file or a web address, themes Dark / Light / Olive, notifications. Window size: setting
`m_iDesktopSize`.

- Desktop icons: apps, then the files and folders in `~/Desktop` (file browsing setting on). Click selects,
  double click or Enter opens, right click for the menu (files: Open, Rename, Delete). Right click on the
  background: Refresh, New folder, New text document, Files, Terminal, Personalize, System info.
- Web access (setting `m_bDesktopWebImages`): each client downloads the address itself (RestApi), decodes
  base64 text and stores media in `$profile:ROOT_AE3/media/`. Images: PNG, JPEG, GIF (first frame), base64
  at most 2 MB. Videos (address ending in mp4, webm, ogv, mov, m4v, avi, mkv, bk2, bik): at most 8 MB,
  played with the engine `VideoWidget`, which refuses formats it does not take. Pages: text, headings,
  lists and links; images and videos on the page are links.
- Windows: drag by the title bar; drop on the top edge to maximize, on the left or right edge to fill that half;
  double click the title bar to maximize or restore; resize from the bottom right corner. Ctrl+W closes.
- Taskbar: start button, pinned Files / Browser / Email / Terminal, open windows (click to focus or minimize,
  right click to close), clock. Start menu: search box (Enter opens the first match), power menu.
- Open / Save dialog (Notepad, Photos, Remote Access): places (Home, This computer, Drives, Temp), Up, New folder,
  click selects, double click opens a folder or picks a file, file name field; Enter accepts, Backspace goes up,
  Esc cancels.
- Keys reach the focused window while no text field is in write mode.

| App | Use |
| --- | --- |
| My Computer | Volumes, flash drive mount and eject |
| Files | Back / forward / up, address bar (click empty space to type a path), search, navigation pane, large icons or details (click a header to sort), right-click menus, new folder / text document, cut / copy / paste, rename, delete to Recycle Bin, shortcuts, properties (owner, permissions), unlock. Keys: Enter, Backspace, Delete, F2, F5, arrows, Ctrl+C / X / V |
| Notepad | New, Open..., Save, Save as... |
| Photos | Open... media files with caption, Web address... (image or video), Paste base64..., Save as... (media file), Set as background (images); videos: Play, Stop, Restart |
| Terminal | Switches to the terminal view |
| Browser | Registered web pages, history; http(s) addresses (web access on): pages as text with links, images and videos in the page |
| Email | Inbox / Sent, search, reading pane, compose (From picks an own address), reply, delete, own addresses (create, delete) |
| Messenger | Message Center (conversations, search, new conversation with a known or typed handle), Handles (create, delete), Reading as (all or one own handle), message bubbles |
| Calendar | Month grid (previous / next month and year, go to date, Today), day panel with events (time, title, location, details), add, delete. Left / Right: month |
| Network | Connection status, networks in range with signal and security, connect (password asked for secured networks), disconnect |
| Remote Access | SSH: remote file list, view, Download... to a local folder, Upload... a local file |
| Ping | Reachability |
| Map | Devices up to 5 km around (same network only with `m_bDesktopMapSameNetworkOnly`); wheel or +/- zooms at the cursor, drag or arrow keys move, Center, scale bar |
| CCTV | Every registered camera (renders through world camera index 3) |
| Games | Game list (installed games), lobby (new table, join, play against bot, own matches), waiting room (seat cards, Start, Add computer, Remove computer, Practice seat), table (drawn board, seat chips, side panel with the game's buttons, typed move field, Switch seat, Leave). Boards: chess and checkers (click a piece, then a marked square; promotion choice), Uno (click a raised card or the draw pile; wilds ask for the colour), Ludo and Snakes and Ladders (die, Roll, click a glowing token), Blackjack (Deal, Hit, Stand), Rock Paper Scissors (click a throw), Battleship (choose ship and direction, click own grid; Random, Clear, Ready; click enemy waters to fire). Practice seats (host, setting `m_bGamesDebugSeats`): this computer holds several seats and plays the one to move; when every seat may act, the next own seat follows each move |
| Snake | In the window; arrow keys or WASD, Enter or Space starts, small / large board, best score; pauses without focus |
| Calculator | + - * / %, sign, backspace; keyboard and numpad (Enter or = result, Backspace, Delete or C clears) |
| Settings | Theme, background (black, 1-5, Picture... from an image file, Web address...), terminal design |
| Recycle Bin, System Info, About | |
