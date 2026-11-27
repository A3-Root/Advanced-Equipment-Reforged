//! armaOS program 'mv': Moves a file or folder.
class ROOT_AER_Cmd_Mv : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "mv";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/mv";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Moves a file or folder.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage mv: 'mv [old path] [new path]' moves file to new path or renames the file.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		if (args.Count() != 2)
		{
			Usage(ctx);
			return;
		}

		string err;
		string src = ctx.Abs(args[0]);
		string dst = ctx.Abs(args[1]);
		if (!ctx.Fs().Move(src, dst, ctx.FsUser(), err))
		{
			ctx.Err(string.Format("mv: cannot move '%1' to '%2': %3", args[0], args[1], err));
			return;
		}

	}
}
