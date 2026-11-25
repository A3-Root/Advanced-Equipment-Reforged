//! armaOS program 'shutdown': Shuts down the computer.
class ROOT_AE3_Cmd_Shutdown : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "shutdown";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/sbin/shutdown";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Shuts down the computer.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage shutdown: 'shutdown' turns off the computer.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		if (ctx.m_Term.InSsh())
		{
			ROOT_AE3_ComputerComponent remote = ctx.m_Exec;
			ROOT_AE3_Shell.EndSsh(ctx.m_Session, "Remote host is shutting down. Connection closed.");
			remote.Shutdown();
			return;
		}

		ctx.Out("System is shutting down...");
		ctx.m_Session.Flush();
		ctx.m_Exec.Shutdown();
	}
}
