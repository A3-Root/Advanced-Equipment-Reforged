//! armaOS program 'rm': Removes a file.
class ROOT_AER_Cmd_Rm : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "rm";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/rm";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Removes a file.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage rm: 'rm [path]' deletes a file at the given path. 'rm -r [path]' also deletes directories.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		ROOT_AER_Opts o = ROOT_AER_Opts.Parse(args, "rf");
		if (!o.m_sError.IsEmpty())
		{
			ctx.Err("rm: " + o.m_sError);
			return;
		}

		if (o.Count() == 0)
		{
			Usage(ctx);
			return;
		}

		bool recursive = o.Has("r");
		foreach (string p : o.m_aArgs)
		{
			string err;
			string abs = ctx.Abs(p);
			ROOT_AER_FsNode node = ctx.Fs().Resolve(abs, ctx.FsUser(), err, false);
			if (node && node.m_bDir && !recursive)
			{
				ctx.Err(string.Format("rm: cannot remove '%1': %2 (use -r)", p, ROOT_AER_Filesystem.ERR_IS_DIR));
				continue;
			}

			if (!ctx.Fs().Remove(abs, ctx.FsUser(), recursive, err))
				ctx.Err(string.Format("rm: cannot remove '%1': %2", p, err));
		}
	}
}
