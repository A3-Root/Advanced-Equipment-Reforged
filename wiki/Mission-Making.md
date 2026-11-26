# Mission making

1. Place device prefabs from `Prefabs/ROOT_AE3/` (World Editor) or the Edit mode list (Game Master).
2. Name the power sources and routers (entity name) and enter the names in `ROOT_AE3_ConnectionComponent`
   of each device to cable them at mission start.
3. Add users, files, mails, web pages and calendar events in `ROOT_AE3_ComputerComponent` and
   `ROOT_AE3_SetupComponent`.
4. Override settings in the mission header field `m_ROOT_AE3_Settings`, or place the AE3 System module
   (`Prefabs/ROOT_AE3/Systems/ROOT_AE3_SystemModule.et`, Game Master Systems) and set its `m_Settings`. The module
   wins over the header; a value set on a device (World Editor attribute other than "Setting default", Game
   Master action or attribute) wins over the module.
5. 77th JSOC EWO mode: `m_bEwoMode` on; EWO packs are the prefabs named in `m_sEwoPrefabs` (default `Radio_ANPRC77`).

File permissions are six characters: owner read, write, execute, then everyone read, write, execute,
`-` for unset (`rw-r--`). Locked files open with `unlock FILE PASSWORD`. Images are files with the content
`AE3_MEDIA|<kind>|<source>|<caption>` and open in Photos: kind `image` (texture resource), `url` (http or https
address of a PNG, JPEG or GIF), `video` (http or https address of a video file) or `base64` (image bytes as base64, `data:` prefix allowed; files hold at most
65536 characters). `AddMedia` and the Game Master "Add image" action pick the kind from the source.
