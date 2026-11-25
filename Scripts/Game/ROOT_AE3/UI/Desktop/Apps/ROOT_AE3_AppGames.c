//! Networked games: lobby and table in a window (same commands as the terminal).
class ROOT_AE3_AppGames : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Games";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "players";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(760, 560, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected int m_iGame;
	protected TextWidget m_wOut;
	protected EditBoxWidget m_wInput;
	protected ref map<string, string> m_mText = new map<string, string>();

	override void Build()
	{
		float w = Width();
		float h = Height();
		array<string> games = ROOT_AE3_GameRules.Games();
		float x = 6;
		foreach (int i, string g : games)
		{
			Color bg = ROOT_AE3_Theme.Button();
			if (i == m_iGame)
				bg = ROOT_AE3_Theme.Accent();

			ROOT_AE3_UI.Button(GetContent(), x, 6, 88, 24, g, 100 + i, this, bg, ROOT_AE3_Theme.Text(), 13);
			x += 92;
		}

		ROOT_AE3_UI.Rect(GetContent(), 6, 36, w - 12, h - 76, ROOT_AE3_Theme.Field());
		m_wOut = Label(12, 40, w - 24, h - 84, "", 14, true, false);
		m_wInput = Field(6, h - 34, w - 100, "", 1);
		Btn(w - 88, h - 34, 82, 26, "Send", 2);
		string game = games[m_iGame];
		if (m_mText.Contains(game))
			m_wOut.SetText(m_mText.Get(game));
		else
			Send("refresh");
	}

	//------------------------------------------------------------------------------------------------
	protected void Send(string line)
	{
		array<string> f = {ROOT_AE3_GameRules.Games()[m_iGame], line};
		ROOT_AE3_Client.Get().Send(ROOT_AE3_EMsg.GAME_INPUT, ROOT_AE3_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	void OnState(string game, int table, string text)
	{
		array<string> lines = {};
		text.Split("\n", lines, false);
		int start = Math.Max(0, lines.Count() - 30);
		string shown;
		for (int i = start; i < lines.Count(); i++)
		{
			shown += lines[i] + "\n";
		}

		m_mText.Set(game, shown);
		if (game == ROOT_AE3_GameRules.Games()[m_iGame] && m_wOut)
			m_wOut.SetText(shown);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (finished && id == 1)
		{
			Send(m_wInput.GetText());
			m_wInput.SetText("");
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= 100 && id < 100 + ROOT_AE3_GameRules.Games().Count())
		{
			m_iGame = id - 100;
			Redraw();
			return true;
		}

		if (id == 2)
		{
			Send(m_wInput.GetText());
			m_wInput.SetText("");
			return true;
		}

		return false;
	}
}
