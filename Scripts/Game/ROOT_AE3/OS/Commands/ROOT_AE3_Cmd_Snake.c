//! armaOS program 'snake': Retro Snake Game
class ROOT_AE3_Cmd_Snake : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "snake";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/games/snake";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Retro Snake Game";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Retro Snake Game - use option --big for doubled block size. Arrow keys or WASD steer, Esc quits.";
	}

	//------------------------------------------------------------------------------------------------
	override bool IsSshCompatible()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		array<string> longs = {"big"};
		ROOT_AE3_Opts o = ROOT_AE3_Opts.Parse(args, "", longs);
		if (!o.m_sError.IsEmpty())
		{
			ctx.Err("snake: " + o.m_sError);
			return;
		}

		ROOT_AE3_Net.SendToPlayer(ctx.m_Session.m_iPlayerId, ROOT_AE3_EMsg.SNAKE, ROOT_AE3_Util.BoolStr(o.Has("big")));
	}
}
