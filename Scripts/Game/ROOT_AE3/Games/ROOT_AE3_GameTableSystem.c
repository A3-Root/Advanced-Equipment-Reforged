//! Server: game tables, seats, turns, timeouts, power draw, updates to the seated computers' sessions.
class ROOT_AE3_GameTableSystem
{
	protected static ref ROOT_AE3_GameTableSystem s_Instance;
	protected ref array<ref ROOT_AE3_GameTable> m_aTables = {};
	protected ref array<ref ROOT_AE3_GameFrontend> m_aFrontends = {};
	protected int m_iNextId = 1;
	protected bool m_bSweeping;

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_GameTableSystem Get()
	{
		if (!s_Instance)
			s_Instance = new ROOT_AE3_GameTableSystem();

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		if (s_Instance)
			GetGame().GetCallqueue().Remove(s_Instance.Sweep);

		s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	protected float Now()
	{
		return GetGame().GetWorld().GetWorldTime() / 1000.0;
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureSweep()
	{
		if (m_bSweeping)
			return;

		m_bSweeping = true;
		GetGame().GetCallqueue().CallLater(Sweep, 5000, true);
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_GameTable Find(int id)
	{
		foreach (ROOT_AE3_GameTable t : m_aTables)
		{
			if (t.m_iId == id)
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void ListTables(string game, ROOT_AE3_ComputerComponent c, notnull array<ROOT_AE3_GameTable> open, notnull array<ROOT_AE3_GameTable> mine)
	{
		foreach (ROOT_AE3_GameTable t : m_aTables)
		{
			if (t.m_sGame != game)
				continue;

			if (t.SeatOf(c) >= 0)
				mine.Insert(t);
			else if (!t.m_bStarted && t.m_aSeats.Count() < t.m_Rules.MaxSeats() && Reachable(c, t))
				open.Insert(t);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool Reachable(ROOT_AE3_ComputerComponent c, ROOT_AE3_GameTable t)
	{
		if (!ROOT_AE3_SettingsManager.Get().m_bGamesRequireSameNetwork)
			return true;

		ROOT_AE3_ComputerComponent host = t.GetHost();
		if (!host || host == c)
			return true;

		int hops;
		return ROOT_AE3_NetworkSystem.Get().CanReach(c.GetNetwork(), host.GetNetwork(), hops);
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_GameTable Host(ROOT_AE3_ComputerComponent c, string game, out string status)
	{
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		if (!s.m_bGamesEnabled)
		{
			status = "Games are disabled on this computer.";
			return null;
		}

		if (!s.m_bGamesAllowHosting)
		{
			status = "Hosting new tables is disabled.";
			return null;
		}

		if (m_aTables.Count() >= s.m_iGamesMaxTables)
		{
			status = "Too many games are already running.";
			return null;
		}

		ROOT_AE3_GameRules rules = ROOT_AE3_GameRules.Create(game);
		if (!rules)
		{
			status = "Unknown game.";
			return null;
		}

		ROOT_AE3_GameTable t = new ROOT_AE3_GameTable();
		t.m_iId = m_iNextId;
		m_iNextId++;
		t.m_sGame = game;
		t.m_Rules = rules;
		t.m_fLastActivity = Now();
		m_aTables.Insert(t);
		Seat(t, c);
		EnsureSweep();
		status = "Table opened. Waiting for another player to join.";
		if (rules.MinSeats() <= 1)
			status = "Table opened. Type start to play.";

		return t;
	}

	//------------------------------------------------------------------------------------------------
	protected void Seat(ROOT_AE3_GameTable t, ROOT_AE3_ComputerComponent c)
	{
		t.m_aSeats.Insert(c);
		t.m_aSeatNames.Insert(c.GetHostname());
		t.m_aPrefs.Insert(0);
		UpdatePower(c);
	}

	//------------------------------------------------------------------------------------------------
	bool Join(ROOT_AE3_ComputerComponent c, int id, out string status)
	{
		ROOT_AE3_GameTable t = Find(id);
		if (!t)
		{
			status = "That table no longer exists.";
			return false;
		}

		if (t.SeatOf(c) >= 0)
		{
			status = "Resumed.";
			return true;
		}

		bool debugSeats = ROOT_AE3_SettingsManager.Get().m_bGamesDebugSeats;
		if (t.m_bStarted)
		{
			status = "That match has already started.";
			return false;
		}

		if (t.m_aSeats.Count() >= t.m_Rules.MaxSeats())
		{
			status = "That table is full.";
			return false;
		}

		if (!Reachable(c, t))
		{
			status = "That table is not reachable from this network.";
			return false;
		}

		Seat(t, c);
		t.m_fLastActivity = Now();
		status = "Seat taken.";
		if (debugSeats)
			status += " Practice seats are allowed.";

		Push(t, "A player joined the table.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Practice seat (DebugSeats setting): the same computer takes another seat.
	bool AddPracticeSeat(ROOT_AE3_ComputerComponent c, int id, out string status)
	{
		ROOT_AE3_GameTable t = Find(id);
		if (!t || t.m_bStarted || t.m_aSeats.Count() >= t.m_Rules.MaxSeats())
		{
			status = "No seat available.";
			return false;
		}

		if (!ROOT_AE3_SettingsManager.Get().m_bGamesDebugSeats)
		{
			status = "Practice seats are switched off on this computer.";
			return false;
		}

		Seat(t, c);
		status = "Practice seat added.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool StartTable(ROOT_AE3_ComputerComponent c, int id, out string status)
	{
		ROOT_AE3_GameTable t = Find(id);
		if (!t)
		{
			status = "That table no longer exists.";
			return false;
		}

		if (t.GetHost() != c)
		{
			status = "Only the computer hosting the table can start it.";
			return false;
		}

		if (t.m_bStarted)
		{
			status = "That match has already started.";
			return false;
		}

		if (t.m_aSeats.Count() < t.m_Rules.MinSeats())
		{
			status = "Not enough players have taken a seat yet.";
			return false;
		}

		ROOT_AE3_GameChess chess = ROOT_AE3_GameChess.Cast(t.m_Rules);
		if (chess && t.m_aPrefs.Count() >= 2)
		{
			if (t.m_aPrefs[0] != 0 && t.m_aPrefs[0] == t.m_aPrefs[1])
			{
				status = "Two players have asked for the same side. The table starts once every choice is a different one.";
				return false;
			}

			chess.m_aSidePref[0] = t.m_aPrefs[0];
			chess.m_aSidePref[1] = t.m_aPrefs[1];
		}

		int starter = t.m_iStarter;
		if (starter < 0 || starter >= t.m_aSeats.Count())
			starter = 0;

		t.m_Rules.Start(t.m_aSeats.Count(), starter);
		t.m_bStarted = true;
		t.m_fLastActivity = Now();
		t.m_fTurnStart = Now();
		foreach (ROOT_AE3_ComputerComponent seatC : t.m_aSeats)
		{
			UpdatePower(seatC);
		}

		status = "The match has started.";
		Push(t, status);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool Move(ROOT_AE3_ComputerComponent c, int id, string move, out string status)
	{
		ROOT_AE3_GameTable t = Find(id);
		if (!t)
		{
			status = "That table no longer exists.";
			return false;
		}

		int seat = t.SeatOf(c);
		if (seat < 0)
		{
			status = "This computer holds no seat at that table.";
			return false;
		}

		if (!t.m_bStarted)
		{
			status = "The match has not started yet.";
			return false;
		}

		// Practice seats: the same computer acts for whichever of its seats is to move.
		int turn = t.m_Rules.GetTurn();
		if (turn >= 0 && turn < t.m_aSeats.Count() && t.m_aSeats[turn] == c)
			seat = turn;

		if (!t.m_Rules.Apply(seat, move, status))
			return false;

		t.m_fLastActivity = Now();
		if (t.m_Rules.GetTurn() != t.m_iLastTurn)
		{
			t.m_iLastTurn = t.m_Rules.GetTurn();
			t.m_fTurnStart = Now();
		}

		Push(t, status);
		if (t.m_Rules.GetWinner() != -1)
		{
			foreach (ROOT_AE3_ComputerComponent seatC : t.m_aSeats)
			{
				UpdatePower(seatC);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Leave(ROOT_AE3_ComputerComponent c, int id)
	{
		ROOT_AE3_GameTable t = Find(id);
		if (!t)
			return;

		int seat = t.SeatOf(c);
		if (seat < 0)
			return;

		if (t.m_bStarted && t.m_Rules.GetWinner() == -1)
		{
			t.m_Rules.OnSeatLeft(seat);
			Push(t, "A player left the table.");
		}

		if (!t.m_bStarted)
		{
			t.m_aSeats.Remove(seat);
			t.m_aSeatNames.Remove(seat);
			t.m_aPrefs.Remove(seat);
		}
		else
		{
			t.m_aSeats[seat] = null;
		}

		UpdatePower(c);
		bool empty = true;
		foreach (ROOT_AE3_ComputerComponent s : t.m_aSeats)
		{
			if (s)
				empty = false;
		}

		if (empty)
			Close(t, "The table was closed.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Close(ROOT_AE3_GameTable t, string reason)
	{
		Push(t, reason);
		m_aTables.RemoveItem(t);
		foreach (ROOT_AE3_ComputerComponent c : t.m_aSeats)
		{
			if (c)
				UpdatePower(c);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Host options: name, starter seat; any seat: seat name, side preference.
	bool SetOption(ROOT_AE3_ComputerComponent c, int id, string key, string value, out string status)
	{
		ROOT_AE3_GameTable t = Find(id);
		int seat = -1;
		if (t)
			seat = t.SeatOf(c);

		if (seat < 0)
		{
			status = "This computer holds no seat at that table.";
			return false;
		}

		value = ROOT_AE3_Codec.Clean(value);
		switch (key)
		{
			case "seatname":
			{
				t.m_aSeatNames[seat] = ROOT_AE3_Util.Clip(ROOT_AE3_Codec.Clean(value), 32);
				status = "Seat renamed.";
				return true;
			}

			case "name":
			{
				if (seat != 0)
				{
					status = "Only the computer hosting the table can change that.";
					return false;
				}

				t.m_sName = ROOT_AE3_Util.Clip(ROOT_AE3_Codec.Clean(value), 32);
				status = "Table renamed.";
				return true;
			}

			case "starter":
			{
				if (seat != 0)
				{
					status = "Only the computer hosting the table can change that.";
					return false;
				}

				t.m_iStarter = value.ToInt() - 1;
				status = "First seat set.";
				return true;
			}

			case "pref":
			{
				int p = 0;
				if (value == "white")
					p = 1;
				else if (value == "black")
					p = 2;
				else if (value != "random")
				{
					status = "Pick white, black or random.";
					return false;
				}

				t.m_aPrefs[seat] = p;
				status = "Side preference set.";
				return true;
			}
		}

		status = "Unknown option.";
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePower(ROOT_AE3_ComputerComponent c)
	{
		if (!c || !c.GetPower())
			return;

		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		float draw;
		foreach (ROOT_AE3_GameTable t : m_aTables)
		{
			if (t.SeatOf(c) < 0)
				continue;

			if (t.m_bStarted && t.m_Rules.GetWinner() == -1)
				draw = Math.Max(draw, s.m_fGamesPowerDraw);
			else
				draw = Math.Max(draw, s.m_fGamesIdleDraw);
		}

		c.GetPower().SetExtraDraw(draw);
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the table view to every front end showing it.
	void Push(ROOT_AE3_GameTable t, string status)
	{
		for (int i = m_aFrontends.Count() - 1; i >= 0; i--)
		{
			ROOT_AE3_GameFrontend f = m_aFrontends[i];
			if (!f || !f.IsAlive())
			{
				m_aFrontends.Remove(i);
				continue;
			}

			if (f.m_iTable == t.m_iId)
				f.OnTableUpdate(t, status);
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddFrontend(ROOT_AE3_GameFrontend f)
	{
		if (!m_aFrontends.Contains(f))
			m_aFrontends.Insert(f);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveFrontend(ROOT_AE3_GameFrontend f)
	{
		m_aFrontends.RemoveItem(f);
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop Games app input: "game|line".
	void OnInput(ROOT_AE3_Session s, string data)
	{
		array<string> f = ROOT_AE3_Codec.Split(data);
		if (f.Count() < 2)
			return;

		if (s.GetTerminal().CurrentUser().IsEmpty() || !ROOT_AE3_Filesystem.ValidName(f[0]))
			return;

		string e;
		ROOT_AE3_FsNode program = s.m_Computer.GetFilesystem().Resolve("/games/" + f[0], "root", e);
		if (!program || program.m_bDir || program.m_sOwner != "root" || (program.m_iPerms & 16) != 0 || !s.m_Computer.IsInstalled(f[0]) || !ROOT_AE3_GameRules.Games().Contains(f[0]))
			return;

		ROOT_AE3_GameFrontend fe;
		foreach (ROOT_AE3_GameFrontend x : m_aFrontends)
		{
			if (x && x.m_bGui && x.m_Session == s && x.m_sGame == f[0])
				fe = x;
		}

		if (!fe)
		{
			fe = new ROOT_AE3_GameFrontend(s, f[0], true);
			AddFrontend(fe);
		}

		fe.OnLine(f[1]);
		fe.FlushGui();
	}

	//------------------------------------------------------------------------------------------------
	void OnSessionClosed(ROOT_AE3_Session s)
	{
		for (int i = m_aFrontends.Count() - 1; i >= 0; i--)
		{
			if (m_aFrontends[i] && m_aFrontends[i].m_Session == s)
				m_aFrontends.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnDeviceOff(ROOT_AE3_ComputerComponent c)
	{
		array<int> ids = {};
		foreach (ROOT_AE3_GameTable t : m_aTables)
		{
			if (t.SeatOf(c) >= 0 && !t.m_bStarted)
				ids.Insert(t.m_iId);
		}

		foreach (int id : ids)
		{
			Leave(c, id);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Sweep()
	{
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		float now = Now();
		for (int i = m_aTables.Count() - 1; i >= 0; i--)
		{
			ROOT_AE3_GameTable t = m_aTables[i];
			if (now - t.m_fLastActivity > s.m_fGamesAbandonTimeout)
			{
				Close(t, "The table was closed after being left alone too long.");
				continue;
			}

			if (t.m_bStarted && t.m_Rules.GetWinner() == -1 && s.m_fGamesTurnTimeout > 0 && now - t.m_fTurnStart > s.m_fGamesTurnTimeout)
			{
				t.m_Rules.OnTimeout();
				t.m_fTurnStart = now;
				t.m_iLastTurn = t.m_Rules.GetTurn();
				Push(t, t.m_Rules.m_sStatus);
			}
		}
	}
}
