//! armaOS program 'umount': Unmounts a flash drive from the specified usb port.
class ROOT_AER_Cmd_Umount : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "umount";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/umount";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Unmounts a flash drive from the specified usb port.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage umount: 'umount [interface name]' unmounts given interface.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		if (args.Count() != 1)
		{
			Usage(ctx);
			return;
		}

		int port = ROOT_AER_Cmd_Lsusb.PortOf(args[0]);
		if (port < 0 || !ctx.m_Exec.Unmount(port))
		{
			ctx.Err(string.Format("umount: %1: not mounted", args[0]));
			return;
		}

		if (ctx.m_Term.GetCwd().StartsWith("/mnt/" + args[0]))
			ctx.m_Term.SetCwd("/mnt");

		ctx.Out(args[0] + " unmounted");
	}
}
