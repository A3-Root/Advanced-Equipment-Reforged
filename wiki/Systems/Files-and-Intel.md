# Files and Intel

## Filesystem

Each computer and flash drive has its own filesystem. Paths start at `/`; users have `/home/<user>` with `Desktop`.
Flash drives mount at `/mnt/usbN`; an empty `/mnt/usbN` takes no files ("No drive in usbN"). Deleted files go to
`/.trash` (root only); the Files app restores or deletes them for their owner or root.

Permissions are six characters: owner read, write, execute, then everyone read, write, execute, `-` for unset
(`rw-r--`). Root and, while `m_bCliElevateSudoers` is on, sudoers ignore permissions.

## Adding files

| Way | Where |
| --- | --- |
| World Editor | `ROOT_AE3_SetupComponent` (computers), `ROOT_AE3_UsbDriveComponent` (flash drives) |
| Game Master | "Add file", "Add directory", "Add image" |
| Script | `ROOT_AE3_API.AddFile`, `AddDirectory`, `AddMedia` |

A file can be locked with a password (`unlock FILE PASSWORD` prints it, `unlock -p` removes the lock), encrypted with
one of 15 ciphers (see [Encryption](Encryption.md)), or both. "Shown size" replaces the size `ls -l` and Files show.

## Images and videos

Media files hold `AE3_MEDIA|<kind>|<source>|<caption>` and open in Photos: kind `image` (texture resource), `url`
(http or https address of a PNG, JPEG or GIF), `video` (http or https address of a video file) or `base64` (image bytes
as base64, `data:` prefix allowed). `AddMedia` and "Add image" pick the kind from the source.

## Mail and chat

Each computer gets the mail address `<hostname>@lan` and the chat handle `@<hostname>` when it has none; users add up
to 16 addresses and 8 handles per computer in Email and Messenger. Mail and chat belong to the computer: every
logged-in user reads them. Mail folders keep 100 mails; a computer sends at most one mail every 2 s. Mail, chat,
`ping`, `ssh` and `msg` need a powered network path between both computers.

## Web pages

`RegisterWebpage` / "Add web page" adds a Markdown page for one computer or every computer; `RegisterSite` adds a site
with several paths. Pages show headings, lists, links, images and videos as links. Browser history entries are added
by the World Editor, Game Master or `AddBrowserHistory`.

## Calendar and CCTV

Calendar events (date, time, title, details, location) come from the World Editor, Game Master or
`AddCalendarEvent`. Cameras registered with `ROOT_AE3_API.RegisterCamera(camera, name)` show in the CCTV app; feeds of
cameras outside the viewer's streaming range stay dark.
