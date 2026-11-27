//! armaOS program 'chown': Changes the owner of a file or directory.
class ROOT_AER_Cmd_Chown : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "chown";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/chown";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Changes the owner of a file or directory.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage chown: 'chown [path] [new owner]' changes the owner.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		if (args.Count() != 2)
		{
			Usage(ctx);
			return;
		}

		if (!ctx.m_Exec.HasUser(args[1]))
		{
			ctx.Err(string.Format("chown: invalid user: '%1'", args[1]));
			return;
		}

		string err;
		if (!ctx.Fs().Chown(ctx.Abs(args[0]), args[1], ctx.FsUser(), err))
			ctx.Err(string.Format("chown: cannot change owner of '%1': %2", args[0], err));
	}
}
