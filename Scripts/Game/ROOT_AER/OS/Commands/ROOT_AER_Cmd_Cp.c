//! armaOS program 'cp': Copies a file or folder.
class ROOT_AER_Cmd_Cp : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "cp";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/cp";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Copies a file or folder.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage cp: 'cp [old path] [new path]' copies file to new path.";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		ROOT_AER_Opts o = ROOT_AER_Opts.Parse(args, "r");
		if (!o.m_sError.IsEmpty() || o.Count() != 2)
		{
			Usage(ctx);
			return;
		}

		string err;
		string src = ctx.Abs(o.Arg(0));
		string dst = ctx.Abs(o.Arg(1));
		ROOT_AER_FsNode node = ctx.Fs().Resolve(src, ctx.FsUser(), err);
		if (!node)
		{
			ctx.Err(string.Format("cp: cannot stat '%1': %2", o.Arg(0), err));
			return;
		}

		float seconds = ROOT_AER_Transfer.Duration(node.GetSize(), ROOT_AER_Transfer.MediumOf(src, dst));
		ROOT_AER_CopyJob job = new ROOT_AER_CopyJob();
		job.m_Ctx = ctx;
		job.m_sSrc = src;
		job.m_sDst = dst;
		job.m_sShownSrc = o.Arg(0);
		job.m_sShownDst = o.Arg(1);
		ctx.Hold();
		if (seconds > 0)
			ctx.Out(string.Format("Copying %1 (%2), about %3 s...", o.Arg(0), ROOT_AER_Util.FormatSize(node.GetSize()), Math.Ceil(seconds)));

		job.Schedule(seconds * 1000);
	}
}
