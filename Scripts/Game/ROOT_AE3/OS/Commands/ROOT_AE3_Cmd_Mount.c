//! armaOS program 'mount': Mounts a flash drive in the specified usb port.
class ROOT_AE3_Cmd_Mount : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "mount";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/mount";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Mounts a flash drive in the specified usb port.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage mount: 'mount [interface name]' mounts given interface (usb0, usb1) at /mnt/[interface name].";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		if (args.Count() != 1)
		{
			Usage(ctx);
			return;
		}

		int port = ROOT_AE3_Cmd_Lsusb.PortOf(args[0]);
		if (port < 0 || port >= ctx.m_Exec.GetUsbPortCount())
		{
			ctx.Err(string.Format("mount: unknown interface '%1'", args[0]));
			return;
		}

		string reason;
		if (!ctx.m_Exec.Mount(port, reason))
		{
			ctx.Err("mount: " + reason);
			return;
		}

		ctx.Out(string.Format("%1 mounted at /mnt/%1", args[0]));
	}
}
