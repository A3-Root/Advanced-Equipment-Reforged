//! Networked games: game list, lobby with open tables and own matches, table with board, moves and controls.
class ROOT_AE3_AppGames : ROOT_AE3_App
{
	protected static const int ID_GAME = 100;
	protected static const int ID_NEW = 1;
	protected static const int ID_REFRESH = 2;
	protected static const int ID_JOIN = 3;
	protected static const int ID_LOBBY = 4;
	protected static const int ID_LEAVE = 5;
	protected static const int ID_START = 6;
	protected static const int ID_PRACTICE = 7;
	protected static const int ID_MOVE = 8;
	protected static const int ID_MOVE_EDIT = 9;
	protected static const int ID_BOT = 10;
	protected static const int ID_UNBOT = 11;
	protected static const int ID_SOLO = 12;
	protected static const int ID_SEAT = 13;
	protected static const float SIDE_W = 180;

	protected ref array<string> m_aIds = {};
	protected ref array<string> m_aTitles = {};
	protected ref array<bool> m_aInstalled = {};
	protected ref array<string> m_aSeats = {};
	protected string m_sGame;
	protected int m_iTable = -1;
	protected ref array<int> m_aTableIds = {};
	protected ref array<bool> m_aMine = {};
	protected ref array<string> m_aView = {};
	protected ROOT_AE3_ListView m_List;
	protected EditBoxWidget m_wMove;
	protected float m_fPoll;

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Games";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "games";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(860, 580, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float h = Height();
		ROOT_AE3_UI.Rect(GetContent(), 0, 0, SIDE_W, h, ROOT_AE3_Theme.Sidebar());
		ROOT_AE3_UI.Text(GetContent(), 14, 12, SIDE_W - 20, 18, "GAMES", 12, ROOT_AE3_Theme.Muted());
		if (m_aIds.IsEmpty())
		{
			Request0("game_list");
		}
		else
		{
			foreach (int i, string id : m_aIds)
			{
				Color bg = ROOT_AE3_UI.Rgb(0, 0, 0, 0);
				if (id == m_sGame)
					bg = ROOT_AE3_Theme.Select();

				Color fg = ROOT_AE3_Theme.Text();
				string label = m_aTitles[i];
				if (!m_aInstalled[i])
				{
					fg = ROOT_AE3_Theme.Muted();
					label += " (not installed)";
				}

				ROOT_AE3_UI.ItemButton(GetContent(), 6, 36 + i * 34, SIDE_W - 12, 32, "games_board", label, ID_GAME + i, this, 14, fg, bg, ROOT_AE3_Theme.Hover());
			}
		}

		if (m_sGame.IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), SIDE_W, h * 0.4, Width() - SIDE_W, 24, "Choose a game on the left.", 16, ROOT_AE3_Theme.Muted(), false, false, 1);
			return;
		}

		if (m_iTable < 0)
			BuildLobby();
		else
			BuildTable();
	}

	//------------------------------------------------------------------------------------------------
	protected string GameTitle()
	{
		int i = m_aIds.Find(m_sGame);
		if (i < 0)
			return m_sGame;

		return m_aTitles[i];
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildLobby()
	{
		float x = SIDE_W + 14;
		float w = Width() - x - 14;
		float h = Height();
		Label(x, 12, w, 30, GameTitle(), 22);
		int gi = m_aIds.Find(m_sGame);
		if (gi >= 0)
			ROOT_AE3_UI.Text(GetContent(), x, 44, w, 18, "Players: " + m_aSeats[gi], 13, ROOT_AE3_Theme.Muted());

		MainBtn(x, 70, 120, 30, "New table", ID_NEW);
		Btn(x + 128, 70, 140, 30, "Join selected", ID_JOIN);
		Btn(x + 276, 70, 90, 30, "Refresh", ID_REFRESH);
		Btn(x + 374, 70, 150, 30, "Play against bot", ID_SOLO);
		m_List = List(1, x, 110, w, h - 120);
		array<string> headers = {"Table", "Host", "Status", "Seats"};
		array<float> widths = {w * 0.34, w * 0.26, w * 0.26, w * 0.14};
		m_List.SetColumns(headers, widths);
		m_List.m_sEmpty = "No tables yet. Press New table to host one.";
		m_List.Render();
		Request1("game_lobby", m_sGame);
	}

	//------------------------------------------------------------------------------------------------
	protected string V(int i)
	{
		return Arg(m_aView, i);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildTable()
	{
		float x = SIDE_W + 14;
		float w = Width() - x - 14;
		float h = Height();
		if (m_aView.IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), x, h * 0.4, w, 24, "Loading table...", 16, ROOT_AE3_Theme.Muted(), false, false, 1);
			return;
		}

		bool started = V(3) == "1";
		bool host = V(4) == "1";
		Label(x, 10, w - 260, 28, V(2), 20);
		ROOT_AE3_UI.Text(GetContent(), x, 38, w - 260, 18, "Seats: " + V(5), 13, ROOT_AE3_Theme.Muted());
		Btn(x + w - 250, 12, 120, 28, "Back to lobby", ID_LOBBY);
		Btn(x + w - 124, 12, 124, 28, "Leave table", ID_LEAVE);
		if (!V(10).IsEmpty())
			MainBtn(x + w - 380, 12, 124, 28, "Switch seat", ID_SEAT);

		float y = 64;
		ROOT_AE3_UI.Box(GetContent(), x, y, w, 34, ROOT_AE3_Theme.Select());
		string line = V(7);
		if (!V(10).IsEmpty())
			line += "   -   " + V(10);

		ROOT_AE3_UI.Text(GetContent(), x + 12, y + 8, w - 24, 20, line, 16, ROOT_AE3_Theme.Text());
		y += 44;
		if (!started)
		{
			if (host)
			{
				MainBtn(x, y, 120, 30, "Start", ID_START);
				Btn(x + 128, y, 110, 30, "Add bot", ID_BOT);
				Btn(x + 246, y, 120, 30, "Remove bot", ID_UNBOT);
				string hint = "Other players open Games on their computer, choose " + GameTitle() + " and join this table. Add bot fills a seat with a computer player.";
				if (V(11) == "1")
				{
					Btn(x + 374, y, 140, 30, "Practice seat", ID_PRACTICE);
					hint += " Practice seat gives this computer another seat: you play every side yourself (Switch seat picks the side when every seat may act).";
				}

				ROOT_AE3_UI.Text(GetContent(), x, y + 40, w, 60, hint, 13, ROOT_AE3_Theme.Muted(), false, true);
			}

			if (!V(9).IsEmpty())
				ROOT_AE3_UI.Text(GetContent(), x, h - 30, w, 20, V(9), 14, ROOT_AE3_Theme.Accent());

			return;
		}

		float boardH = h - y - 110;
		ROOT_AE3_UI.Box(GetContent(), x, y, w, boardH, ROOT_AE3_Theme.Field());
		ROOT_AE3_UI.Text(GetContent(), x + 12, y + 8, w - 24, boardH - 16, V(6), 15, ROOT_AE3_Theme.Text(), true, false);
		y += boardH + 8;
		ROOT_AE3_UI.Text(GetContent(), x, y, w, 34, V(8), 13, ROOT_AE3_Theme.Muted(), false, true);
		y += 38;
		ROOT_AE3_UI.Text(GetContent(), x, y + 5, 50, 20, "Move", 14, ROOT_AE3_Theme.Text());
		m_wMove = Field(x + 50, y, w - 180, "", ID_MOVE_EDIT);
		MainBtn(x + w - 122, y, 122, 26, "Play", ID_MOVE);
		ROOT_AE3_UI.Placeholder(m_wMove, GetContent(), x + 54, y + 4, w - 188, "Type a move, then Enter", ROOT_AE3_Theme.Muted());
		if (!V(9).IsEmpty())
		{
			Color col = ROOT_AE3_Theme.Accent();
			if (V(9).StartsWith("!"))
				col = ROOT_AE3_Theme.Danger();

			ROOT_AE3_UI.Text(GetContent(), x, y + 32, w, 20, V(9), 13, col);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		if (m_iTable < 0)
			return;

		m_fPoll += dt;
		if (m_fPoll > 4)
		{
			m_fPoll = 0;
			Request1("game_view", m_iTable.ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Table update pushed by the server (GAME_STATE).
	void OnState(string game, int table, string text)
	{
		if (table == m_iTable && table >= 0)
			Request1("game_view", m_iTable.ToString());
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			if (m_iTable >= 0 && (cmd == "game_view" || cmd == "game_join"))
			{
				m_iTable = -1;
				m_aView.Clear();
				Redraw();
			}

			return;
		}

		switch (cmd)
		{
			case "game_list":
			{
				m_aIds.Clear();
				m_aTitles.Clear();
				m_aInstalled.Clear();
				m_aSeats.Clear();
				foreach (string p : payload)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(p);
					if (e.Count() < 5)
						continue;

					m_aIds.Insert(e[0]);
					m_aTitles.Insert(e[1]);
					m_aInstalled.Insert(e[2] == "1");
					string seats = e[3];
					if (e[4] != e[3])
						seats += "-" + e[4];

					m_aSeats.Insert(seats);
				}

				if (m_sGame.IsEmpty())
				{
					int first = m_aInstalled.Find(true);
					if (first >= 0)
						m_sGame = m_aIds[first];
				}

				Redraw();
				return;
			}

			case "game_lobby":
			{
				if (Arg(payload, 0) != m_sGame || !m_List)
					return;

				m_List.Clear();
				m_aTableIds.Clear();
				m_aMine.Clear();
				for (int i = 1; i < payload.Count(); i++)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(payload[i]);
					if (e.Count() < 6)
						continue;

					bool mine = e[4] == "1";
					string name = e[1];
					if (mine)
						name += "  (your seat)";

					array<string> row = {name, e[5], e[2], e[3]};
					m_List.AddRow(row, "games_board");
					m_aTableIds.Insert(e[0].ToInt());
					m_aMine.Insert(mine);
				}

				m_List.Render();
				Status(string.Format("%1 tables", m_aTableIds.Count()));
				return;
			}

			case "game_leave":
			{
				m_iTable = -1;
				m_aView.Clear();
				Status(Arg(payload, 0));
				Redraw();
				return;
			}

			case "game_view":
			{
				m_aView.Clear();
				m_aView.Copy(payload);
				m_iTable = V(0).ToInt();
				m_sGame = V(1);
				m_fPoll = 0;
				string keep;
				if (m_wMove && V(9).StartsWith("!"))
					keep = m_wMove.GetText();

				Redraw();
				if (m_wMove && !keep.IsEmpty())
					ROOT_AE3_UI.SetEdit(m_wMove, keep);

				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinSelected()
	{
		if (!m_List)
			return;

		int i = m_List.m_iSelected;
		if (i < 0 || i >= m_aTableIds.Count())
		{
			Status("Select a table first.");
			return;
		}

		Request1("game_join", m_aTableIds[i].ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void SendMove()
	{
		if (!m_wMove || m_wMove.GetText().Trim().IsEmpty())
			return;

		Request2("game_move", m_iTable.ToString(), m_wMove.GetText());
	}

	//------------------------------------------------------------------------------------------------
	override void OnListActivate(int listId, int row)
	{
		JoinSelected();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnKey(KeyCode key, bool ctrl)
	{
		if (key == KeyCode.KC_F5)
		{
			Redraw();
			return true;
		}

		if ((key == KeyCode.KC_RETURN || key == KeyCode.KC_NUMPADENTER) && m_iTable < 0)
		{
			JoinSelected();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (finished && id == ID_MOVE_EDIT)
		{
			SendMove();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= ID_GAME && id < ID_GAME + m_aIds.Count())
		{
			int i = id - ID_GAME;
			if (!m_aInstalled[i])
			{
				Status(m_aTitles[i] + " is not installed on this computer.");
				return true;
			}

			m_sGame = m_aIds[i];
			m_iTable = -1;
			m_aView.Clear();
			Redraw();
			return true;
		}

		string t = m_iTable.ToString();
		switch (id)
		{
			case ID_NEW: Request1("game_new", m_sGame); return true;
			case ID_REFRESH: Redraw(); return true;
			case ID_JOIN: JoinSelected(); return true;
			case ID_LOBBY:
			{
				m_iTable = -1;
				m_aView.Clear();
				Redraw();
				return true;
			}

			case ID_LEAVE: Request1("game_leave", t); return true;
			case ID_START: Request1("game_start", t); return true;
			case ID_PRACTICE: Request1("game_practice", t); return true;
			case ID_SEAT: Request2("game_seat", t, "0"); return true;
			case ID_BOT: Request1("game_bot", t); return true;
			case ID_UNBOT: Request1("game_unbot", t); return true;
			case ID_SOLO: Request1("game_solo", m_sGame); return true;
			case ID_MOVE: SendMove(); return true;
		}

		return false;
	}
}
