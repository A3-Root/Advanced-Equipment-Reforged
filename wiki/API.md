# Script API

All calls on the authority. Devices are entities. Computers accept calls once set up: use
`ROOT_AE3_API.GetOnDeviceReady()` (`device, restored`) for devices created at mission start; it fires again after a
persistence or Game Master restore with `restored` true, so add content only when `restored` is false. Custom commands
(`AddCustomCommand` with a device) are not saved: add them on every call, restored or not.
`GetOnFileChanged` (`computer, path`) fires for every file and folder change on a computer's own filesystem
except log, mail and chat writes.

```c
IEntity laptop = GetGame().GetWorld().FindEntityByName("laptop1");
ROOT_AE3_API.AddUser(laptop, "smith", "1234", true);
ROOT_AE3_API.AddFile(laptop, "/home/smith/orders.txt", "Move at dawn.", "smith", "rw----");
ROOT_AE3_API.AddFile(laptop, "/home/smith/codes.txt", "ALPHA", "smith", "rw----", 1, "3", "secret");
ROOT_AE3_API.RegisterWebpage("intel.local", "Intel", "# Intel\n\n[Home](rootnet.home)");
ROOT_AE3_API.GetOnLogin().Insert(OnLogin);
```

Custom program:

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

With a device the command belongs to that computer only; with `null` it is registered for every computer and
installed with `InstallCommand`. Programs run only on computers they were installed on. Commands and event
subscribers are dropped when the game ends.

## Desktop requests from other mods

Server: `ROOT_AE3_API.RegisterGuiCommand("myapp_get", new MyCommand())` with `MyCommand : ROOT_AE3_GuiCommand`
overriding `Handle(session, rid, cmd, fields, user)` and replying with `ROOT_AE3_GuiRouter.Reply` or `Msg`. Client:
the app class registered with `RegisterExtApp` sends `Request("myapp_get", args)` and receives `OnGuiReply`.

## Network hooks

`ROOT_AE3_API.AddConnectCondition(new MyCondition())` with `MyCondition : ROOT_AE3_ConnectCondition` overriding
`bool Allow(client, router, out reason)`: checked on every connect and wireless sweep; false blocks or drops the
link. `ROOT_AE3_API.AddRangeModifier(new MyModifier())` with `MyModifier : ROOT_AE3_RangeModifier` overriding
`float Modify(router, range)`: applied to every wireless range check (scan, connect, sweep).

## Access conditions

`ROOT_AE3_API.AddAccessCondition(new MyAccess())` with `MyAccess : ROOT_AE3_AccessCondition` overriding
`bool Allow(playerId, computer, mode)` (mode 0 terminal, 1 desktop): checked on every open and every request after
the access lists of the computer; false denies access.
