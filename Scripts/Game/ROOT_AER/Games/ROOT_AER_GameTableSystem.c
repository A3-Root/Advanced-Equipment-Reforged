//! Server: game tables, seats, turns, timeouts, power draw, updates to the seated computers' sessions.
class ROOT_AER_GameTableSystem
{
	protected static ref ROOT_AER_GameTableSystem s_Instance;
	protected ref array<ref ROOT_AER_GameTable> m_aTables = {};
	protected ref array<ref ROOT_AER_GameFrontend> m_aFrontends = {};
	protected int m_iNextId = 1;
	protected bool m_bSweeping;

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_GameTableSystem Get()
	{
		if (!s_Instance)
			s_Instance = new ROOT_AER_GameTableSystem();

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		if (s_Instance)
		{
			GetGame().GetCallqueue().Remove(s_Instance.Sweep);
			GetGame().GetCallqueue().Remove(s_Instance.BotStep);
		}

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
	ROOT_AER_GameTable Find(int id)
	{
		foreach (ROOT_AER_GameTable t : m_aTables)
		{
			if (t.m_iId == id)
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void ListTables(string game, ROOT_AER_ComputerComponent c, notnull array<ROOT_AER_GameTable> open, notnull array<ROOT_AER_GameTable> mine)
	{
		foreach (ROOT_AER_GameTable t : m_aTables)
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
	protected bool Reachable(ROOT_AER_ComputerComponent c, ROOT_AER_GameTable t)
	{
		if (!ROOT_AER_SettingsManager.Get().m_bGamesRequireSameNetwork)
			return true;

		ROOT_AER_ComputerComponent host = t.GetHost();
		if (!host || host == c)
			return true;

		int hops;
		return ROOT_AER_NetworkSystem.Get().CanReach(c.GetNetwork(), host.GetNetwork(), hops);
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AER_GameTable Host(ROOT_AER_ComputerComponent c, string game, out string status)
	{
		ROOT_AER_Settings s = ROOT_AER_SettingsManager.Get();
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

		ROOT_AER_GameRules rules = ROOT_AER_GameRules.Create(game);
		if (!rules)
		{
			status = "Unknown game.";
			return null;
		}

		ROOT_AER_GameTable t = new ROOT_AER_GameTable();
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
	protected void Seat(ROOT_AER_GameTable t, ROOT_AER_ComputerComponent c)
	{
		t.m_aSeats.Insert(c);
		t.m_aSeatNames.Insert(c.GetHostname());
		t.m_aPrefs.Insert(0);
		t.m_aBots.Insert(false);
		UpdatePower(c);
	}

	//------------------------------------------------------------------------------------------------
	//! Host only, before the start: a seat played by the server (setting m_bGamesBots).
	bool AddBot(ROOT_AER_ComputerComponent c, int id, out string status)
	{
		ROOT_AER_GameTable t = Find(id);
		if (!t || t.m_bStarted || t.m_aSeats.Count() >= t.m_Rules.MaxSeats())
		{
			status = "No seat available.";
			return false;
		}

		if (t.GetHost() != c)
		{
			status = "Only the computer hosting the table can add a bot.";
			return false;
		}

		if (!ROOT_AER_SettingsManager.Get().m_bGamesBots)
		{
			status = "Bots are switched off on this server.";
			return false;
		}

		int n = 1;
		foreach (bool b : t.m_aBots)
		{
			if (b)
				n++;
		}

		t.m_aSeats.Insert(null);
		t.m_aSeatNames.Insert("Bot " + n.ToString());
		t.m_aPrefs.Insert(0);
		t.m_aBots.Insert(true);
		t.m_fLastActivity = Now();
		status = "A bot took a seat.";
		Push(t, status);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Host only, before the start: removes the last bot seat.
	bool RemoveBot(ROOT_AER_ComputerComponent c, int id, out string status)
	{
		ROOT_AER_GameTable t = Find(id);
		if (!t || t.m_bStarted || t.GetHost() != c)
		{
			status = "Only the computer hosting the table can remove a bot before the start.";
			return false;
		}

		for (int i = t.m_aBots.Count() - 1; i >= 0; i--)
		{
			if (!t.m_aBots[i])
				continue;

			t.m_aSeats.Remove(i);
			t.m_aSeatNames.Remove(i);
			t.m_aPrefs.Remove(i);
			t.m_aBots.Remove(i);
			status = "Bot removed.";
			Push(t, status);
			return true;
		}

		status = "There is no bot at this table.";
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Queues one bot move when a bot seat has something to do.
	protected void ScheduleBots(ROOT_AER_GameTable t)
	{
		if (!t || t.m_bBotQueued || !t.m_bStarted || t.m_Rules.GetWinner() != -1 || !t.m_aBots.Contains(true))
			return;

		t.m_bBotQueued = true;
		GetGame().GetCallqueue().CallLater(BotStep, 900, false, t.m_iId);
	}

	//------------------------------------------------------------------------------------------------
	protected void BotStep(int id)
	{
		ROOT_AER_GameTable t = Find(id);
		if (!t)
			return;

		t.m_bBotQueued = false;
		if (!t.m_bStarted || t.m_Rules.GetWinner() != -1)
			return;

		int turn = t.m_Rules.GetTurn();
		for (int seat = 0; seat < t.m_aSeats.Count(); seat++)
		{
			if (!t.IsBot(seat) || (turn >= 0 && seat != turn))
				continue;

			array<string> moves = {};
			t.m_Rules.BotMoves(seat, moves);
			foreach (string move : moves)
			{
				string status;
				if (!t.m_Rules.Apply(seat, move, status))
					continue;

				AfterMove(t, t.m_aSeatNames[seat] + ": " + status);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Turn clock, updates, power after the end, next bot move.
	protected void AfterMove(ROOT_AER_GameTable t, string status)
	{
		t.m_fLastActivity = Now();
		if (t.m_Rules.GetTurn() != t.m_iLastTurn)
		{
			t.m_iLastTurn = t.m_Rules.GetTurn();
			t.m_fTurnStart = Now();
		}

		Push(t, status);
		if (t.m_Rules.GetWinner() != -1)
		{
			foreach (ROOT_AER_ComputerComponent seatC : t.m_aSeats)
			{
				UpdatePower(seatC);
			}
		}

		ScheduleBots(t);
	}

	//------------------------------------------------------------------------------------------------
	bool Join(ROOT_AER_ComputerComponent c, int id, out string status)
	{
		ROOT_AER_GameTable t = Find(id);
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

		bool debugSeats = ROOT_AER_SettingsManager.Get().m_bGamesDebugSeats;
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
	bool AddPracticeSeat(ROOT_AER_ComputerComponent c, int id, out string status)
	{
		ROOT_AER_GameTable t = Find(id);
		if (!t || t.m_bStarted || t.m_aSeats.Count() >= t.m_Rules.MaxSeats())
		{
			status = "No seat available.";
			return false;
		}

		if (t.GetHost() != c)
		{
			status = "Only the computer hosting the table can add a practice seat.";
			return false;
		}

		if (!ROOT_AER_SettingsManager.Get().m_bGamesDebugSeats)
		{
			status = "Practice seats are switched off on this computer.";
			return false;
		}

		Seat(t, c);
		int seat = t.m_aSeats.Count() - 1;
		t.m_aSeatNames[seat] = "Practice " + (seat + 1).ToString();
		t.m_fLastActivity = Now();
		status = "Practice seat added. This computer plays every seat it holds.";
		Push(t, status);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A computer holding several seats chooses the one it plays (seat number, or 0 for the next one).
	bool SwitchSeat(ROOT_AER_ComputerComponent c, int id, int seatNumber, out string status)
	{
		ROOT_AER_GameTable t = Find(id);
		if (!t || t.SeatOf(c) < 0)
		{
			status = "This computer holds no seat at that table.";
			return false;
		}

		bool ok;
		if (seatNumber <= 0)
			ok = t.NextPlaySeat(c);
		else
			ok = t.SetPlaySeat(c, seatNumber - 1);

		if (!ok)
		{
			status = "This computer does not hold that seat.";
			return false;
		}

		status = string.Format("Playing seat %1 (%2).", t.PlaySeat(c) + 1, t.m_aSeatNames[t.PlaySeat(c)]);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool StartTable(ROOT_AER_ComputerComponent c, int id, out string status)
	{
		ROOT_AER_GameTable t = Find(id);
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

		ROOT_AER_GameChess chess = ROOT_AER_GameChess.Cast(t.m_Rules);
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
		foreach (ROOT_AER_ComputerComponent seatC : t.m_aSeats)
		{
			UpdatePower(seatC);
		}

		status = "The match has started.";
		Push(t, status);
		ScheduleBots(t);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool Move(ROOT_AER_ComputerComponent c, int id, string move, out string status)
	{
		ROOT_AER_GameTable t = Find(id);
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

		// Practice seats: the seat to move when this computer holds it, else the chosen seat.
		int turn = t.m_Rules.GetTurn();
		seat = t.PlaySeat(c);
		if (!t.m_Rules.Apply(seat, move, status))
			return false;

		// Every seat may act (throws, fleet setup): the next own seat that still has to act is played.
		if (turn < 0 && t.m_Rules.GetTurn() < 0)
		{
			array<int> own = {};
			t.SeatsOf(c, own);
			if (own.Count() > 1)
			{
				t.NextPlaySeat(c);
				status += string.Format(" Now playing seat %1.", t.PlaySeat(c) + 1);
			}
		}

		AfterMove(t, status);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Leave(ROOT_AER_ComputerComponent c, int id)
	{
		ROOT_AER_GameTable t = Find(id);
		if (!t)
			return;

		// Every seat the computer holds goes, practice seats included.
		array<int> seats = {};
		t.SeatsOf(c, seats);
		if (seats.IsEmpty())
			return;

		for (int i = seats.Count() - 1; i >= 0; i--)
		{
			int seat = seats[i];
			if (t.m_bStarted && t.m_Rules.GetWinner() == -1)
				t.m_Rules.OnSeatLeft(seat);

			if (!t.m_bStarted)
			{
				t.m_aSeats.Remove(seat);
				t.m_aSeatNames.Remove(seat);
				t.m_aPrefs.Remove(seat);
				t.m_aBots.Remove(seat);
			}
			else
			{
				t.m_aSeats[seat] = null;
			}
		}

		int ai = t.m_aActiveOwner.Find(c);
		if (ai >= 0)
		{
			t.m_aActiveOwner.Remove(ai);
			t.m_aActiveSeat.Remove(ai);
		}

		if (t.m_bStarted)
			Push(t, "A player left the table.");

		UpdatePower(c);
		bool empty = true;
		foreach (ROOT_AER_ComputerComponent s : t.m_aSeats)
		{
			if (s)
				empty = false;
		}

		if (empty)
			Close(t, "The table was closed.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Close(ROOT_AER_GameTable t, string reason)
	{
		Push(t, reason);
		m_aTables.RemoveItem(t);
		foreach (ROOT_AER_ComputerComponent c : t.m_aSeats)
		{
			if (c)
				UpdatePower(c);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Host options: name, starter seat; any seat: seat name, side preference.
	bool SetOption(ROOT_AER_ComputerComponent c, int id, string key, string value, out string status)
	{
		ROOT_AER_GameTable t = Find(id);
		int seat = -1;
		if (t)
			seat = t.SeatOf(c);

		if (seat < 0)
		{
			status = "This computer holds no seat at that table.";
			return false;
		}

		value = ROOT_AER_Codec.Clean(value);
		switch (key)
		{
			case "seatname":
			{
				t.m_aSeatNames[t.PlaySeat(c)] = ROOT_AER_Util.Clip(ROOT_AER_Codec.Clean(value), 32);
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

				t.m_sName = ROOT_AER_Util.Clip(ROOT_AER_Codec.Clean(value), 32);
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

				t.m_aPrefs[t.PlaySeat(c)] = p;
				status = "Side preference set.";
				return true;
			}
		}

		status = "Unknown option.";
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePower(ROOT_AER_ComputerComponent c)
	{
		if (!c || !c.GetPower())
			return;

		ROOT_AER_Settings s = ROOT_AER_SettingsManager.Get();
		float draw;
		foreach (ROOT_AER_GameTable t : m_aTables)
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
	void Push(ROOT_AER_GameTable t, string status)
	{
		for (int i = m_aFrontends.Count() - 1; i >= 0; i--)
		{
			ROOT_AER_GameFrontend f = m_aFrontends[i];
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
	void AddFrontend(ROOT_AER_GameFrontend f)
	{
		if (!m_aFrontends.Contains(f))
			m_aFrontends.Insert(f);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveFrontend(ROOT_AER_GameFrontend f)
	{
		m_aFrontends.RemoveItem(f);
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop Games app input: "game|line".
	void OnInput(ROOT_AER_Session s, string data)
	{
		array<string> f = ROOT_AER_Codec.Split(data);
		if (f.Count() < 2)
			return;

		if (s.GetTerminal().CurrentUser().IsEmpty() || !Installed(s.m_Computer, f[0]))
			return;

		ROOT_AER_GameFrontend fe = GuiFrontend(s, f[0]);
		fe.OnLine(f[1]);
		fe.FlushGui();
	}

	//------------------------------------------------------------------------------------------------
	//! The game exists and its program /games/<game> is installed unmodified on the computer.
	bool Installed(ROOT_AER_ComputerComponent c, string game)
	{
		if (!c || !c.GetFilesystem() || !ROOT_AER_Filesystem.ValidName(game) || !ROOT_AER_GameRules.Games().Contains(game) || !c.IsInstalled(game))
			return false;

		string e;
		ROOT_AER_FsNode program = c.GetFilesystem().Resolve("/games/" + game, "root", e);
		return program && !program.m_bDir && program.m_sOwner == "root" && (program.m_iPerms & 16) == 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop front end of a session for one game; receives table updates as GAME_STATE.
	ROOT_AER_GameFrontend GuiFrontend(ROOT_AER_Session s, string game)
	{
		foreach (ROOT_AER_GameFrontend x : m_aFrontends)
		{
			if (x && x.m_bGui && x.m_Session == s && x.m_sGame == game)
				return x;
		}

		ROOT_AER_GameFrontend fe = new ROOT_AER_GameFrontend(s, game, true);
		AddFrontend(fe);
		return fe;
	}

	//------------------------------------------------------------------------------------------------
	void OnSessionClosed(ROOT_AER_Session s)
	{
		for (int i = m_aFrontends.Count() - 1; i >= 0; i--)
		{
			if (m_aFrontends[i] && m_aFrontends[i].m_Session == s)
				m_aFrontends.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnDeviceOff(ROOT_AER_ComputerComponent c)
	{
		array<int> ids = {};
		foreach (ROOT_AER_GameTable t : m_aTables)
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
		ROOT_AER_Settings s = ROOT_AER_SettingsManager.Get();
		float now = Now();
		for (int i = m_aTables.Count() - 1; i >= 0; i--)
		{
			ROOT_AER_GameTable t = m_aTables[i];
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
				ScheduleBots(t);
			}
		}
	}
}
