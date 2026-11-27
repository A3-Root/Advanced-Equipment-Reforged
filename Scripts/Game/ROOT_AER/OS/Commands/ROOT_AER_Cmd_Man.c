//! armaOS program 'man': Prints usage information about a command.
class ROOT_AER_Cmd_Man : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "man";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/man";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Prints usage information about a command.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage man: 'man [command]' returns usage information for the command.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		if (args.IsEmpty())
		{
			Usage(ctx);
			return;
		}

		ROOT_AER_Command c = ROOT_AER_Cmd_Help.FindInstalled(ctx, args[0]);
		if (!c)
		{
			ctx.Err("No manual entry for " + args[0]);
			return;
		}

		ctx.Out(c.GetManual());
	}
}
