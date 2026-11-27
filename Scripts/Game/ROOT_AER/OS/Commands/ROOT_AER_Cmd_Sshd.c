//! armaOS program 'sshd': Shows or sets whether this computer accepts SSH logins (remote access).
class ROOT_AER_Cmd_Sshd : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "sshd";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/sbin/sshd";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Shows or sets remote access (SSH logins) of this computer.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage sshd: 'sshd' or 'sshd status' shows whether this computer accepts SSH logins. 'sshd on' and 'sshd off' set it (root or sudo).";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		ROOT_AER_NetworkComponent n = ctx.m_Exec.GetNetwork();
		if (!n || n.IsRouter())
		{
			ctx.Err("sshd: no network interface");
			return;
		}

		string arg = "status";
		if (args.Count() == 1)
			arg = args[0];
		else if (args.Count() > 1)
		{
			Usage(ctx);
			return;
		}

		if (arg == "status")
		{
			if (n.IsSshEnabled())
				ctx.Out("sshd: remote access on");
			else
				ctx.Out("sshd: remote access off");

			return;
		}

		if (arg != "on" && arg != "off")
		{
			Usage(ctx);
			return;
		}

		if (ctx.m_sUser != "root")
		{
			ctx.Err("sshd: permission denied (use sudo)");
			return;
		}

		n.SetSshEnabled(arg == "on");
		ctx.Out("sshd: remote access " + arg);
	}
}
