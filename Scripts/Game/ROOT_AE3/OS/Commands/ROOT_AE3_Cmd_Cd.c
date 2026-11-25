//! armaOS program 'cd': Change the working directory.
class ROOT_AE3_Cmd_Cd : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "cd";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/cd";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Change the working directory.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage cd: 'cd [path]' sets path as the new working directory.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		string target = "~";
		if (!args.IsEmpty())
			target = args[0];

		string abs = ctx.Abs(target);
		string err;
		ROOT_AE3_FsNode node = ctx.Fs().Resolve(abs, ctx.FsUser(), err);
		if (!node)
		{
			ctx.Err(string.Format("cd: %1: %2", target, err));
			return;
		}

		if (!node.m_bDir)
		{
			ctx.Err(string.Format("cd: %1: %2", target, ROOT_AE3_Filesystem.ERR_NOT_DIR));
			return;
		}

		if (!node.HasPermission(ctx.FsUser(), 2))
		{
			ctx.Err(string.Format("cd: %1: %2", target, ROOT_AE3_Filesystem.ERR_PERMISSION));
			return;
		}

		ctx.m_Term.SetCwd(abs);
	}
}
