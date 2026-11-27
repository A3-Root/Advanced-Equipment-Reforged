# Extending AE3

Classes use the prefix `ROOT_AE3_`. Change vanilla behaviour only with `modded class`. State changes run on the
authority; see [Limits and Locality](../Reference/Limits-and-Locality.md).

## Custom programs

```c
class MyCallback : ROOT_AE3_CommandCallback
{
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ctx.Out("hello " + ctx.m_sUser);
	}
}

ROOT_AE3_API.AddCustomCommand(laptop, "hello", "Greets", "Usage hello: 'hello'", new MyCallback());
```

With a device the program belongs to that computer only; with `null` it is registered for every computer and
installed with `InstallCommand(device, name)`. Programs run only on computers they were installed on. Programs and
event subscribers are dropped when the game ends. `ctx.Err` prints an error, `ctx.Fs()` is the filesystem,
`ctx.Abs(path)` resolves a path.

## Desktop apps

Client: subclass `ROOT_AE3_App` (override `GetTitle`, `GetIcon`, `GetSize`, `Build`, `OnGuiReply`, `OnUiClick`) and
register it with `ROOT_AE3_API.RegisterExtApp(MyApp)`. Builders are in `ROOT_AE3_UI` and the app helpers (`Btn`,
`MainBtn`, `Card`, `Heading`, `KeyValue`, `Meter`, `ToolbarBg`); colours come from `ROOT_AE3_Theme`, which follows the
computer's style.

Server: `ROOT_AE3_API.RegisterGuiCommand("myapp_get", new MyCommand())` with `MyCommand : ROOT_AE3_GuiCommand`
overriding `Handle(session, rid, cmd, fields, user)` and replying with `ROOT_AE3_GuiRouter.Reply` or `Msg`. The app
sends `Request1("myapp_get", arg)` and receives `OnGuiReply`.

## Network hooks

- `ROOT_AE3_API.AddConnectCondition(new MyCondition())`, `MyCondition : ROOT_AE3_ConnectCondition` overriding
  `bool Allow(client, router, out reason)`: checked on every connect and wireless sweep; false blocks or drops the link.
- `ROOT_AE3_API.AddRangeModifier(new MyModifier())`, `MyModifier : ROOT_AE3_RangeModifier` overriding
  `float Modify(router, range)`: applied to every wireless range check.

## Access conditions

`ROOT_AE3_API.AddAccessCondition(new MyAccess())`, `MyAccess : ROOT_AE3_AccessCondition` overriding
`bool Allow(playerId, computer, mode)` (mode 0 terminal, 1 desktop): checked on every open and request after the
access lists; false denies access.

## Ciphers

`ROOT_AE3_Encryption.Apply(id, text, key, decrypt)` runs any cipher; `Name`, `FromName`, `Label`, `KeyHint`,
`DefaultKey` and `CheckKey` describe them.

## Assets

Models are built by `Assets/ROOT_AE3/Source/build_models.py` in Blender (`blender --background --python
build_models.py -- <model>`) and imported by Workbench. Sounds are synthesized by
`Sounds/ROOT_AE3/Source/make_sounds.py`. Resource GUIDs and `.meta` files come from Workbench only.

## Validation

The project must compile in Arma Reforger Tools without script errors or warnings. Record each change as one line in
`releases/CHANGELOG.md` and update the wiki page of the system it touches.
