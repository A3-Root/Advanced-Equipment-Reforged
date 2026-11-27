//! armaOS program of a networked game (/games/<id>): opens the game lobby in the terminal.
class ROOT_AER_Cmd_Game : ROOT_AER_Command
{
	protected string m_sGame;
	protected string m_sTitle;

	//------------------------------------------------------------------------------------------------
	void ROOT_AER_Cmd_Game(string game)
	{
		m_sGame = game;
		ROOT_AER_GameRules r = ROOT_AER_GameRules.Create(game);
		if (r)
			m_sTitle = r.GetTitle();
	}

	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return m_sGame;
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/games/" + m_sGame;
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return m_sTitle + " (network game)";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return string.Format("Usage %1: '%1' opens the lobby. Lobby: new, join <number>, resume <number>, refresh, quit. At a table: moves, board, status, seatname <text>, leave, quit.", m_sGame);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsSshCompatible()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AER_ShellContext ctx, array<string> args)
	{
		if (!ROOT_AER_SettingsManager.Get().m_bGamesEnabled)
		{
			ctx.Err("Games are disabled on this computer.");
			return;
		}

		ROOT_AER_GameFrontend fe = new ROOT_AER_GameFrontend(ctx.m_Session, m_sGame, false);
		ROOT_AER_GameTableSystem.Get().AddFrontend(fe);
		ROOT_AER_GameTui app = new ROOT_AER_GameTui();
		app.m_Frontend = fe;
		ctx.m_Term.m_App = app;
		fe.ShowLobby();
	}
}
