//! armaOS program 'touch': Creates an empty file.
class ROOT_AER_Cmd_Touch : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "touch";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/touch";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Creates an empty file.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage touch: 'touch [filename]' creates an empty file with the given name.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		if (args.IsEmpty())
		{
			Usage(ctx);
			return;
		}

		foreach (string p : args)
		{
			string abs = ctx.Abs(p);
			if (ctx.Fs().Exists(abs))
				continue;

			string err;
			if (!ctx.Fs().WriteFile(abs, "", ctx.FsUser(), false, err, -1, ctx.m_sUser))
				ctx.Err(string.Format("touch: cannot touch '%1': %2", p, err));
		}
	}
}
