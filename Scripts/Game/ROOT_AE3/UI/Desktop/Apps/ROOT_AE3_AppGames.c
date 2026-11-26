//! Networked games: game list, lobby with open tables and own matches, waiting room, and the table with a
//! graphical board (ROOT_AE3_GameBoard), a side panel with the controls and a move field.
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
	protected static const float PANEL_W = 290;
	protected static const int VIEW_FIELDS = 13;

	protected ref array<string> m_aIds = {};
	protected ref array<string> m_aTitles = {};
	protected ref array<bool> m_aInstalled = {};
	protected ref array<string> m_aSeats = {};
	protected string m_sGame;
	protected int m_iTable = -1;
	protected ref array<int> m_aTableIds = {};
	protected ref array<bool> m_aMine = {};
	protected ref array<string> m_aView = {};
	protected ref ROOT_AE3_GameBoard m_Board;
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
		return Vector(1100, 720, 0);
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

				ROOT_AE3_UI.ItemButton(GetContent(), 6, 36 + i * 38, SIDE_W - 12, 34, "games_board", label, ID_GAME + i, this, 15, fg, bg, ROOT_AE3_Theme.Hover());
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
	protected void BoardFor(string game)
	{
		if (!m_Board || m_Board.m_sGame != game)
			m_Board = ROOT_AE3_GameBoard.Create(game, this);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildLobby()
	{
		float x = SIDE_W + 16;
		float w = Width() - x - 16;
		float h = Height();

		// Banner with a picture of the game.
		float bannerH = 120;
		ROOT_AE3_UI.Rect(GetContent(), x, 12, w, bannerH, ROOT_AE3_UI.Rgb(0.11, 0.2, 0.3));
		ROOT_AE3_UI.Rect(GetContent(), x, 12 + bannerH - 4, w, 4, ROOT_AE3_Theme.Accent());
		BoardFor(m_sGame);
		m_Board.Begin(GetContent(), Width(), h);
		m_Board.Preview(x + w - 300, 22, 280, bannerH - 30);
		ROOT_AE3_UI.Text(GetContent(), x + 20, 30, w - 340, 36, GameTitle(), 30, ROOT_AE3_UI.Rgb(1, 1, 1));
		int gi = m_aIds.Find(m_sGame);
		if (gi >= 0)
			ROOT_AE3_UI.Text(GetContent(), x + 20, 74, w - 340, 20, "Players: " + m_aSeats[gi], 15, ROOT_AE3_UI.Rgb(0.75, 0.82, 0.9));

		float y = 12 + bannerH + 16;
		MainBtn(x, y, 220, 40, "Play against the computer", ID_SOLO);
		Btn(x + 230, y, 160, 40, "Host a new table", ID_NEW);
		Btn(x + 400, y, 150, 40, "Join selected", ID_JOIN);
		Btn(x + 560, y, 100, 40, "Refresh", ID_REFRESH);
		y += 54;
		ROOT_AE3_UI.Text(GetContent(), x, y, w, 20, "Open tables and your matches", 14, ROOT_AE3_Theme.Muted());
		y += 24;
		m_List = List(1, x, y, w, h - y - 12);
		array<string> headers = {"Table", "Host", "Status", "Seats"};
		array<float> widths = {w * 0.34, w * 0.26, w * 0.26, w * 0.14};
		m_List.SetColumns(headers, widths);
		m_List.m_sEmpty = "No tables yet. Play against the computer, or host a table for other players.";
		m_List.Render();
		m_Board.End();
		Request1("game_lobby", m_sGame);
	}

	//------------------------------------------------------------------------------------------------
	protected string V(int i)
	{
		return Arg(m_aView, i);
	}

	//------------------------------------------------------------------------------------------------
	//! Seat names sent after the fixed fields of game_view.
	protected array<string> SeatNames()
	{
		array<string> names = {};
		for (int i = VIEW_FIELDS; i < m_aView.Count(); i++)
		{
			names.Insert(m_aView[i]);
		}

		return names;
	}

	//------------------------------------------------------------------------------------------------
	//! Header: table name, seat chips, buttons; banner with the state line. Returns the y below.
	protected float BuildHeader(float x, float w)
	{
		Label(x, 10, w - 420, 30, V(2), 22);
		Btn(x + w - 260, 12, 126, 30, "Back to lobby", ID_LOBBY);
		Btn(x + w - 128, 12, 128, 30, "Leave table", ID_LEAVE);
		if (!V(10).IsEmpty())
			MainBtn(x + w - 400, 12, 132, 30, "Switch seat", ID_SEAT);

		array<string> names = SeatNames();
		int n = names.Count();
		if (n > 0)
		{
			float chipW = (w - 10) / n;
			if (chipW > 190)
				chipW = 190;

			foreach (int i, string name : names)
			{
				float cx = x + i * chipW;
				bool turn = m_Board && m_Board.HasState() && m_Board.m_iTurn == i && m_Board.m_iWinner == -1;
				Color edge = ROOT_AE3_Theme.Border();
				if (turn)
					edge = ROOT_AE3_Theme.Accent();

				ROOT_AE3_UI.Box(GetContent(), cx, 46, chipW - 8, 30, ROOT_AE3_Theme.Field(), edge);
				if (turn)
					ROOT_AE3_UI.Rect(GetContent(), cx, 46, chipW - 8, 3, ROOT_AE3_Theme.Accent());

				int col = 0xFF808080;
				if (m_Board)
					col = m_Board.SeatColor(i);

				ROOT_AE3_UI.Rect(GetContent(), cx + 8, 55, 12, 12, Color.FromInt(col));
				string label = name;
				if (m_Board && m_Board.HasState())
				{
					if (m_Board.IsBot(i))
						label += " (bot)";
					else if (m_Board.IsMine(i))
						label += " (here)";
				}

				ROOT_AE3_UI.Text(GetContent(), cx + 26, 52, chipW - 40, 20, label, 14, ROOT_AE3_Theme.Text());
			}
		}

		// State banner.
		string line = V(7);
		Color band = ROOT_AE3_Theme.Select();
		if (line == "Your turn." || line == "Every player may act now.")
			band = ROOT_AE3_Theme.Accent();
		else if (line == "You won!")
			band = ROOT_AE3_UI.Rgb(0.2, 0.6, 0.3);
		else if (line == "You lost.")
			band = ROOT_AE3_Theme.Danger();

		ROOT_AE3_UI.Rect(GetContent(), x, 84, w, 38, band);
		if (!V(10).IsEmpty())
			line += "    " + V(10);

		ROOT_AE3_UI.Text(GetContent(), x + 14, 92, w - 28, 22, line, 17, ROOT_AE3_UI.Rgb(1, 1, 1));
		return 132;
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

		BoardFor(V(1));
		m_Board.SetState(V(12), SeatNames());
		float y = BuildHeader(x, w);
		if (V(3) != "1")
		{
			BuildWaiting(x, y, w, h);
			return;
		}

		float boardW = w - PANEL_W - 12;
		float boardH = h - y - 10;
		float px = x + boardW + 12;
		ROOT_AE3_UI.Box(GetContent(), px, y, PANEL_W, boardH, ROOT_AE3_Theme.Field());
		m_Board.Begin(GetContent(), Width(), h);
		if (m_Board.HasState())
			m_Board.Build(x, y, boardW, boardH);

		float used = m_Board.BuildSide(px + 14, y + 14, PANEL_W - 28);
		float cy = y + 14 + used;
		if (used > 0)
			cy += 10;

		string hint = m_Board.Hint();
		if (!hint.IsEmpty())
		{
			ROOT_AE3_UI.Text(GetContent(), px + 14, cy, PANEL_W - 28, 60, hint, 14, ROOT_AE3_Theme.Accent(), false, true);
			cy += 64;
		}

		if (!V(9).IsEmpty())
		{
			Color col = ROOT_AE3_Theme.Text();
			if (V(9).StartsWith("!"))
				col = ROOT_AE3_Theme.Danger();

			ROOT_AE3_UI.Text(GetContent(), px + 14, cy, PANEL_W - 28, 60, V(9), 14, col, false, true);
		}

		// Rules and the move field at the bottom of the panel.
		float by = y + boardH - 146;
		ROOT_AE3_UI.Rect(GetContent(), px + 14, by, PANEL_W - 28, 1, ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.Text(GetContent(), px + 14, by + 8, PANEL_W - 28, 70, V(8), 12, ROOT_AE3_Theme.Muted(), false, true);
		m_wMove = Field(px + 14, by + 84, PANEL_W - 28, "", ID_MOVE_EDIT);
		ROOT_AE3_UI.Placeholder(m_wMove, GetContent(), px + 18, by + 88, PANEL_W - 36, "Or type a move, then Enter", ROOT_AE3_Theme.Muted());
		MainBtn(px + 14, by + 114, PANEL_W - 28, 26, "Play typed move", ID_MOVE);
		m_Board.End();
	}

	//------------------------------------------------------------------------------------------------
	//! Before the start: seat cards and the host's controls.
	protected void BuildWaiting(float x, float y, float w, float h)
	{
		bool host = V(4) == "1";
		array<string> names = SeatNames();
		float cw = 200;
		float cy = y + 10;
		ROOT_AE3_UI.Text(GetContent(), x, cy, w, 22, "Seats", 16, ROOT_AE3_Theme.Text());
		cy += 30;
		int perRow = (w + 10) / (cw + 10);
		if (perRow < 1)
			perRow = 1;

		foreach (int i, string name : names)
		{
			int col = i % perRow;
			int row = i / perRow;
			float sx = x + col * (cw + 10);
			float sy = cy + row * 80;
			ROOT_AE3_UI.Box(GetContent(), sx, sy, cw, 70, ROOT_AE3_Theme.Field());
			ROOT_AE3_UI.Rect(GetContent(), sx, sy, 6, 70, Color.FromInt(m_Board.SeatColor(i)));
			ROOT_AE3_UI.Text(GetContent(), sx + 18, sy + 10, cw - 26, 22, name, 17, ROOT_AE3_Theme.Text());
			string role = "Player";
			if (i == 0)
				role = "Host";
			else if (name.StartsWith("Bot "))
				role = "Computer player";
			else if (name.StartsWith("Practice "))
				role = "Practice seat (played here)";

			ROOT_AE3_UI.Text(GetContent(), sx + 18, sy + 40, cw - 26, 18, role, 13, ROOT_AE3_Theme.Muted());
		}

		int rows = (names.Count() + perRow - 1) / perRow;
		cy += rows * 80 + 16;
		if (host)
		{
			MainBtn(x, cy, 160, 44, "Start the match", ID_START);
			Btn(x + 170, cy, 150, 44, "Add computer", ID_BOT);
			Btn(x + 330, cy, 160, 44, "Remove computer", ID_UNBOT);
			string hint = "Other players open Games on their computer, choose " + GameTitle() + " and join this table. Add computer fills a seat with a computer player.";
			if (V(11) == "1")
			{
				Btn(x + 500, cy, 150, 44, "Practice seat", ID_PRACTICE);
				hint += " Practice seat gives this computer another seat: you play every side yourself.";
			}

			ROOT_AE3_UI.Text(GetContent(), x, cy + 56, w, 60, hint, 14, ROOT_AE3_Theme.Muted(), false, true);
		}

		if (!V(9).IsEmpty())
			ROOT_AE3_UI.Text(GetContent(), x, h - 30, w, 20, V(9), 14, ROOT_AE3_Theme.Accent());
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
		if (!m_wMove)
			return;

		string move = m_wMove.GetText();
		move.TrimInPlace();
		if (!move.IsEmpty())
			Request2("game_move", m_iTable.ToString(), move);
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
		if (id >= ROOT_AE3_GameBoard.ID_BASE && m_Board && m_iTable >= 0)
		{
			bool redraw;
			string move = m_Board.Click(id, redraw);
			if (!move.IsEmpty())
				Request2("game_move", m_iTable.ToString(), move);

			if (redraw)
				Redraw();

			return true;
		}

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
