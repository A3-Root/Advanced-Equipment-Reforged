//! Text front end of one game for one session: lobby and table commands. Prints to the terminal
//! or, for the desktop Games app, collects the text and sends it as GAME_STATE.
class ROOT_AER_GameFrontend
{
	ROOT_AER_Session m_Session;
	string m_sGame;
	int m_iTable = -1;
	bool m_bGui;
	protected ref array<string> m_aGuiOut = {};

	//------------------------------------------------------------------------------------------------
	void ROOT_AER_GameFrontend(ROOT_AER_Session session, string game, bool gui)
	{
		m_Session = session;
		m_sGame = game;
		m_bGui = gui;
	}

	//------------------------------------------------------------------------------------------------
	bool IsAlive()
	{
		return m_Session && m_Session.IsUsable();
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AER_ComputerComponent Computer()
	{
		return m_Session.m_Computer;
	}

	//------------------------------------------------------------------------------------------------
	protected void Out(string text)
	{
		if (m_bGui)
			m_aGuiOut.Insert(text);
		else
			m_Session.Out(text);
	}

	//------------------------------------------------------------------------------------------------
	void FlushGui()
	{
		if (!m_bGui)
			return;

		string text;
		foreach (int i, string l : m_aGuiOut)
		{
			if (i > 0)
				text += "\n";

			text += l;
		}

		m_aGuiOut.Clear();
		array<string> f = {m_sGame, m_iTable.ToString(), text};
		ROOT_AER_Net.SendToPlayer(m_Session.m_iPlayerId, ROOT_AER_EMsg.GAME_STATE, ROOT_AER_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	void ShowLobby()
	{
		ROOT_AER_GameRules r = ROOT_AER_GameRules.Create(m_sGame);
		string title = m_sGame;
		if (r)
			title = r.GetTitle();

		Out(string.Format("== %1 - Lobby ==", title));
		if (ROOT_AER_SettingsManager.Get().m_bGamesRequireSameNetwork)
		{
			ROOT_AER_NetworkComponent n = Computer().GetNetwork();
			if (!n || n.GetAddress().IsEmpty())
				Out("This computer is not on a network.");
		}

		array<ROOT_AER_GameTable> open = {};
		array<ROOT_AER_GameTable> mine = {};
		ROOT_AER_GameTableSystem.Get().ListTables(m_sGame, Computer(), open, mine);
		Out("Your matches:");
		if (mine.IsEmpty())
			Out("  none");

		foreach (ROOT_AER_GameTable t : mine)
		{
			Out("  " + t.Label());
		}

		Out("Open tables:");
		if (open.IsEmpty())
			Out("  none");

		foreach (ROOT_AER_GameTable t : open)
		{
			Out("  " + t.Label());
		}

		Out("Commands: new, join <number>, resume <number>, refresh, quit");
	}

	//------------------------------------------------------------------------------------------------
	void ShowTable(ROOT_AER_GameTable t, string status = "")
	{
		int seat = t.SeatOf(Computer());
		Out("== " + t.Label() + " ==");
		string seats;
		foreach (int i, string n : t.m_aSeatNames)
		{
			if (i > 0)
				seats += ", ";

			seats += string.Format("%1 %2", i + 1, n);
		}

		Out("Seats: " + seats);
		if (!t.m_bStarted)
		{
			if (seat == 0)
				Out("Type start when every player has joined. Options: bot, unbot, name <text>, seatname <text>, pref white|black|random, starter <seat>, practice.");
			else
				Out("Waiting for the host to start. Options: seatname <text>, pref white|black|random.");
		}
		else
		{
			int turn = t.m_Rules.GetTurn();
			array<int> own = {};
			t.SeatsOf(Computer(), own);
			if (own.Count() > 1)
				Out(string.Format("Playing seat %1 (%2). Type seat <number> or seat to switch.", t.PlaySeat(Computer()) + 1, t.m_aSeatNames[t.PlaySeat(Computer())]));

			Out(t.m_Rules.View(t.PlaySeat(Computer())));
			int w = t.m_Rules.GetWinner();
			if (w == -2)
				Out("The match is a draw.");
			else if (w >= 0 && t.m_aSeats.IsIndexValid(w) && t.m_aSeats[w] == Computer())
				Out("You won.");
			else if (w >= 0)
				Out("You lost.");
			else if (turn < 0)
				Out("Every seat may act. " + t.m_Rules.GetHelp());
			else if (t.m_aSeats.IsIndexValid(turn) && t.m_aSeats[turn] == Computer())
				Out("Your turn. " + t.m_Rules.GetHelp());
			else
				Out("Waiting for the other player.");
		}

		if (!status.IsEmpty())
			Out("> " + status);

		Out("Commands: a move, board, status, seat <number>, seatname <text>, leave, quit");
	}

	//------------------------------------------------------------------------------------------------
	void OnTableUpdate(ROOT_AER_GameTable t, string status)
	{
		ShowTable(t, status);
		if (m_bGui)
		{
			FlushGui();
		}
		else
		{
			m_Session.SendPrompt();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns false when the player quit.
	bool OnLine(string line)
	{
		ROOT_AER_GameTableSystem sys = ROOT_AER_GameTableSystem.Get();
		line = line.Trim();
		array<string> w = ROOT_AER_Util.Words(line);
		string verb;
		if (!w.IsEmpty())
		{
			verb = w[0];
			verb.ToLower();
		}

		string rest = ROOT_AER_Util.JoinFrom(w, 1);
		string status;
		if (verb == "quit" || verb == "exit")
		{
			sys.RemoveFrontend(this);
			Out("Bye.");
			return false;
		}

		if (m_iTable < 0)
		{
			switch (verb)
			{
				case "new":
				{
					ROOT_AER_GameTable t = sys.Host(Computer(), m_sGame, status);
					if (t)
					{
						m_iTable = t.m_iId;
						ShowTable(t, status);
					}
					else
					{
						Out(status);
					}

					return true;
				}

				case "join":
				case "resume":
				{
					int id = rest.ToInt();
					if (sys.Join(Computer(), id, status))
					{
						m_iTable = id;
						ShowTable(sys.Find(id), status);
					}
					else
					{
						Out(status);
					}

					return true;
				}

				case "":
				case "refresh":
				case "help":
				{
					ShowLobby();
					return true;
				}
			}

			Out("Unknown command.");
			ShowLobby();
			return true;
		}

		ROOT_AER_GameTable table = sys.Find(m_iTable);
		if (!table)
		{
			Out("That table no longer exists.");
			m_iTable = -1;
			ShowLobby();
			return true;
		}

		switch (verb)
		{
			case "":
			case "board":
			case "status":
			{
				ShowTable(table);
				return true;
			}

			case "leave":
			{
				sys.Leave(Computer(), m_iTable);
				m_iTable = -1;
				ShowLobby();
				return true;
			}

			case "lobby":
			{
				m_iTable = -1;
				ShowLobby();
				return true;
			}

			case "start":
			{
				if (!sys.StartTable(Computer(), m_iTable, status))
					Out(status);

				return true;
			}

			case "practice":
			{
				sys.AddPracticeSeat(Computer(), m_iTable, status);
				Out(status);
				return true;
			}

			case "seat":
			{
				sys.SwitchSeat(Computer(), m_iTable, rest.ToInt(), status);
				Out(status);
				ShowTable(table);
				return true;
			}

			case "bot":
			{
				sys.AddBot(Computer(), m_iTable, status);
				Out(status);
				return true;
			}

			case "unbot":
			{
				sys.RemoveBot(Computer(), m_iTable, status);
				Out(status);
				return true;
			}

			case "name":
			case "seatname":
			case "pref":
			case "starter":
			{
				sys.SetOption(Computer(), m_iTable, verb, rest, status);
				Out(status);
				return true;
			}
		}

		if (!sys.Move(Computer(), m_iTable, line, status))
			Out(status);

		return true;
	}
}
