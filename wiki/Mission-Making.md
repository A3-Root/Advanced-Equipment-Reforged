# Mission making

1. Place device prefabs from `Prefabs/ROOT_AE3/` (World Editor) or the Edit mode list (Game Master).
2. Name the power sources and routers (entity name) and enter the names in `ROOT_AE3_ConnectionComponent`
   of each device to cable them at mission start.
3. Add users, files, mails, web pages and calendar events in `ROOT_AE3_ComputerComponent` and
   `ROOT_AE3_SetupComponent`.
4. Override settings in the mission header field `m_ROOT_AE3_Settings`.

File permissions are six characters: owner read, write, execute, then everyone read, write, execute,
`-` for unset (`rw-r--`). Locked files open with `unlock FILE PASSWORD`. Images are files with the content
`AE3_MEDIA|image|<texture resource>|<caption>` and open in the Image Viewer.
