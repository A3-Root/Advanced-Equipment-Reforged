//! armaOS program 'unlock': Open a password-protected file.
class ROOT_AE3_Cmd_Unlock : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "unlock";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/unlock";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Open a password-protected file.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage unlock: 'unlock FILEPATH PASSWORD' prints the protected content. 'unlock -p FILEPATH PASSWORD' permanently removes the protection (requires write permission).";
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		ROOT_AE3_Opts o = ROOT_AE3_Opts.Parse(args, "p");
		if (!o.m_sError.IsEmpty() || o.Count() != 2)
		{
			Usage(ctx);
			return;
		}

		string abs = ctx.Abs(o.Arg(0));
		string content, err;
		if (!ctx.Fs().ReadFile(abs, ctx.FsUser(), content, err))
		{
			ctx.Err(string.Format("unlock: %1: %2", o.Arg(0), err));
			return;
		}

		if (!ROOT_AE3_Intel.IsLocked(content))
		{
			ctx.Err(string.Format("unlock: %1: file is not protected", o.Arg(0)));
			return;
		}

		string payload;
		if (!ctx.m_Session.CheckAuth(ROOT_AE3_Intel.Unlock(content, o.Arg(1), payload)))
		{
			ctx.Err("unlock: wrong password");
			ctx.Log("/var/log/auth.log", string.Format("unlock: failed attempt on '%1' by '%2'", abs, ctx.m_sUser));
			return;
		}

		if (o.Has("p"))
		{
			if (!ctx.Fs().WriteFile(abs, payload, ctx.FsUser(), false, err))
			{
				ctx.Err(string.Format("unlock: %1: %2", o.Arg(0), err));
				return;
			}

			ctx.Out(string.Format("Protection removed from %1.", o.Arg(0)));
			return;
		}

		ctx.Out(payload);
	}
}
