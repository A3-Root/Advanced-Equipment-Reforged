//! armaOS program 'find': Searches in the current directory for a file/folder with the given name.
class ROOT_AE3_Cmd_Find : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "find";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/find";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Searches in the current directory for a file/folder with the given name.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage find: 'find [Name]' prints the results to stdout. Name is a regular expression matched against file names.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		if (args.IsEmpty())
		{
			Usage(ctx);
			return;
		}

		array<string> results = {};
		ctx.Fs().Find(ctx.Cwd(), args[0], ctx.FsUser(), results);
		if (results.IsEmpty())
		{
			ctx.Out("No results.");
			return;
		}

		foreach (string r : results)
		{
			ctx.Out(r);
		}
	}
}
