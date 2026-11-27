//! armaOS program 'sudo': Execute a command as root.
class ROOT_AER_Cmd_Sudo : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "sudo";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/sudo";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Execute a command as root.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage sudo: 'sudo COMMAND ARGS...' executes the command as root. The current user must be listed in /etc/sudoers.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		if (args.IsEmpty())
		{
			Usage(ctx);
			return;
		}

		if (!ctx.m_Exec.IsSudoer(ctx.m_sUser))
		{
			ctx.Err(string.Format("%1 is not in the sudoers file. This incident will be reported.", ctx.m_sUser));
			ctx.Log("/var/log/auth.log", string.Format("sudo: denied for user '%1'", ctx.m_sUser));
			return;
		}

		string name = args[0];
		array<string> rest = {};
		for (int i = 1; i < args.Count(); i++)
		{
			rest.Insert(args[i]);
		}

		ctx.Log("/var/log/auth.log", string.Format("sudo: user '%1' executed '%2'", ctx.m_sUser, ROOT_AER_Util.JoinFrom(args, 0)));
		string original = ctx.m_sUser;
		ctx.m_sUser = "root";
		ctx.m_bSudo = true;
		ROOT_AER_Shell.Execute(ctx, name, rest);
		if (!ctx.IsHeld())
		{
			ctx.m_sUser = original;
			ctx.m_bSudo = false;
		}
	}
}
