//! armaOS program 'mkdir': Creates a directory/folder.
class ROOT_AE3_Cmd_Mkdir : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "mkdir";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/mkdir";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Creates a directory/folder.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage mkdir: 'mkdir [path]' creates a new folder/directory. 'mkdir -p' creates missing parents.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ROOT_AE3_Opts o = ROOT_AE3_Opts.Parse(args, "p");
		if (!o.m_sError.IsEmpty() || o.Count() == 0)
		{
			Usage(ctx);
			return;
		}

		foreach (string p : o.m_aArgs)
		{
			string err;
			if (!ctx.Fs().CreateDir(ctx.Abs(p), ctx.FsUser(), ctx.m_sUser, ROOT_AE3_FsNode.PERM_DIR_DEFAULT, err, o.Has("p")))
				ctx.Err(string.Format("mkdir: cannot create directory '%1': %2", p, err));
		}
	}
}
