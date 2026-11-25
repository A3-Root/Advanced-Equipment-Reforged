//! armaOS program 'grep': Search file contents for lines matching a pattern.
class ROOT_AE3_Cmd_Grep : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "grep";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/grep";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Search file contents for lines matching a pattern.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage grep: 'grep -i -v PATTERN FILEPATH...' prints lines of the given files matching the regular expression PATTERN. -i ignores case, -v inverts the match.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ROOT_AE3_Opts o = ROOT_AE3_Opts.Parse(args, "iv");
		if (!o.m_sError.IsEmpty())
		{
			ctx.Err("grep: " + o.m_sError);
			return;
		}

		if (o.Count() < 2)
		{
			Usage(ctx);
			return;
		}

		string pattern = o.Arg(0);
		bool ignoreCase = o.Has("i");
		bool invert = o.Has("v");
		bool prefix = o.Count() > 2;
		for (int i = 1; i < o.Count(); i++)
		{
			string p = o.Arg(i);
			string content, err;
			if (!ctx.Fs().ReadFile(ctx.Abs(p), ctx.FsUser(), content, err))
			{
				ctx.Err(string.Format("grep: %1: %2", p, err));
				continue;
			}

			if (ROOT_AE3_Intel.IsLocked(content))
				continue;

			array<string> lines = {};
			content.Split("\n", lines, false);
			foreach (string l : lines)
			{
				bool hit = ROOT_AE3_Regex.Search(pattern, l, ignoreCase);
				if (ROOT_AE3_Regex.LimitHit())
					break;

				if (hit != invert)
				{
					if (prefix)
						ctx.Out(p + ":" + l);
					else
						ctx.Out(l);
				}
			}
		}

		if (ROOT_AE3_Regex.LimitHit())
			ctx.Err("grep: search limit reached");
	}
}
