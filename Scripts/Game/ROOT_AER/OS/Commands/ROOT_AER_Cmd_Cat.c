//! armaOS program 'cat': Prints the content of a file.
class ROOT_AER_Cmd_Cat : ROOT_AER_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "cat";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/cat";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Prints the content of a file.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage cat: 'cat [file]' shows the content of a file.";
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
			if (abs == "/sys/battery/capacity")
				ctx.m_Exec.UpdateBatteryFile();

			string content, err;
			if (!ctx.Fs().ReadFile(abs, ctx.FsUser(), content, err))
			{
				ctx.Err(string.Format("cat: %1: %2", p, err));
				continue;
			}

			if (ROOT_AER_Intel.IsLocked(content))
			{
				ctx.Err(string.Format("cat: %1: file is password protected; use 'unlock %1 PASSWORD'", p));
				continue;
			}

			if (ROOT_AER_Intel.IsMedia(content))
			{
				ctx.Out(string.Format("%1: media file (%2); open it in the desktop Image Viewer", p, ROOT_AER_Intel.MediaType(content)));
				continue;
			}

			if (!ROOT_AER_CommandRegistry.ProgramOf(content).IsEmpty())
			{
				ctx.Out(string.Format("%1: binary program", p));
				continue;
			}

			ctx.Out(content);
		}
	}
}
