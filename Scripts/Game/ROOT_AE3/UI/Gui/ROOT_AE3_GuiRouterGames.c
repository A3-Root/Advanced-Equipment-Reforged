//! Server: desktop Games app requests. Lobby rows and table views as fields; table changes are pushed
//! to the session as GAME_STATE through its desktop front end.
class ROOT_AE3_GuiRouterGames
{
	//------------------------------------------------------------------------------------------------
	protected static string A(array<string> f, int i)
	{
		if (i < f.Count())
			return f[i];

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static bool Handle(ROOT_AE3_Session s, string rid, string cmd, array<string> f, string user)
	{
		if (!cmd.StartsWith("game_"))
			return false;

		ROOT_AE3_ComputerComponent c = s.m_Computer;
		ROOT_AE3_GameTableSystem sys = ROOT_AE3_GameTableSystem.Get();
		if (!ROOT_AE3_SettingsManager.Get().m_bGamesEnabled)
		{
			ROOT_AE3_GuiRouter.Msg(s, rid, false, cmd, "Games are switched off on this server.");
			return true;
		}

		if (cmd == "game_list")
		{
			array<string> res = {};
			foreach (string id : ROOT_AE3_GameRules.Games())
			{
				ROOT_AE3_GameRules r = ROOT_AE3_GameRules.Create(id);
				array<string> e = {id, r.GetTitle(), ROOT_AE3_Util.BoolStr(sys.Installed(c, id)), r.MinSeats().ToString(), r.MaxSeats().ToString()};
				res.Insert(ROOT_AE3_Codec.JoinSub(e));
			}

			ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, res);
			return true;
		}

		string status;
		if (cmd == "game_lobby" || cmd == "game_new" || cmd == "game_solo")
		{
			string game = A(f, 2);
			if (!sys.Installed(c, game))
			{
				ROOT_AE3_GuiRouter.Msg(s, rid, false, cmd, "This game is not installed on this computer.");
				return true;
			}

			if (cmd == "game_new" || cmd == "game_solo")
			{
				ROOT_AE3_GameTable nt = sys.Host(c, game, status);
				if (!nt)
				{
					ROOT_AE3_GuiRouter.Msg(s, rid, false, cmd, status);
					return true;
				}

				sys.GuiFrontend(s, game).m_iTable = nt.m_iId;
				if (cmd == "game_solo")
				{
					if (sys.AddBot(c, nt.m_iId, status))
						sys.StartTable(c, nt.m_iId, status);
					else
						status = "! " + status;
				}

				View(s, rid, nt, status);
				return true;
			}

			array<ROOT_AE3_GameTable> open = {};
			array<ROOT_AE3_GameTable> mine = {};
			sys.ListTables(game, c, open, mine);
			array<string> rows = {game};
			foreach (ROOT_AE3_GameTable t : mine)
			{
				rows.Insert(Row(t, true));
			}

			foreach (ROOT_AE3_GameTable t : open)
			{
				rows.Insert(Row(t, false));
			}

			ROOT_AE3_GuiRouter.Reply(s, rid, true, cmd, rows);
			return true;
		}

		int tableId = A(f, 2).ToInt();
		ROOT_AE3_GameTable table = sys.Find(tableId);
		if (!table || !sys.Installed(c, table.m_sGame))
		{
			ROOT_AE3_GuiRouter.Msg(s, rid, false, cmd, "That table no longer exists.");
			return true;
		}

		bool ok = true;
		switch (cmd)
		{
			case "game_view": break;
			case "game_join":
			{
				ok = sys.Join(c, tableId, status);
				if (ok)
					sys.GuiFrontend(s, table.m_sGame).m_iTable = tableId;

				break;
			}

			case "game_leave":
			{
				sys.Leave(c, tableId);
				sys.GuiFrontend(s, table.m_sGame).m_iTable = -1;
				ROOT_AE3_GuiRouter.Msg(s, rid, true, cmd, "You left the table.");
				return true;
			}

			case "game_start": ok = sys.StartTable(c, tableId, status); break;
			case "game_practice": ok = sys.AddPracticeSeat(c, tableId, status); break;
			case "game_bot": ok = sys.AddBot(c, tableId, status); break;
			case "game_seat": ok = sys.SwitchSeat(c, tableId, A(f, 3).ToInt(), status); break;
			case "game_unbot": ok = sys.RemoveBot(c, tableId, status); break;
			case "game_move": ok = sys.Move(c, tableId, A(f, 3), status); break;
			case "game_option": ok = sys.SetOption(c, tableId, A(f, 3), A(f, 4), status); break;
			default:
			{
				ROOT_AE3_GuiRouter.Msg(s, rid, false, cmd, "Unknown request " + cmd);
				return true;
			}
		}

		// The table may have closed (last seat left, match over).
		table = sys.Find(tableId);
		if (!table)
		{
			ROOT_AE3_GuiRouter.Msg(s, rid, false, cmd, "That table no longer exists.");
			return true;
		}

		if (!ok && table.SeatOf(c) < 0)
		{
			ROOT_AE3_GuiRouter.Msg(s, rid, false, cmd, status);
			return true;
		}

		if (!ok)
			status = "! " + status;

		View(s, rid, table, status);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Lobby row: id, name, state, seats, mine.
	protected static string Row(ROOT_AE3_GameTable t, bool mine)
	{
		string name = t.m_sName;
		if (name.IsEmpty())
			name = t.m_Rules.GetTitle() + " #" + t.m_iId;

		string state = "Waiting for players";
		if (t.m_bStarted)
			state = "Playing";

		if (t.m_Rules.GetWinner() != -1)
			state = "Finished";

		string host = "";
		if (t.GetHost())
			host = t.GetHost().GetHostname();

		array<string> e = {t.m_iId.ToString(), name, state, string.Format("%1/%2", t.m_aSeats.Count(), t.m_Rules.MaxSeats()), ROOT_AE3_Util.BoolStr(mine), host};
		return ROOT_AE3_Codec.JoinSub(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Table view reply "game_view": id, game, name, started, host, seats, board, line, help, status, playing, practice allowed.
	static void View(ROOT_AE3_Session s, string rid, ROOT_AE3_GameTable t, string status)
	{
		ROOT_AE3_ComputerComponent c = s.m_Computer;
		int seat = t.SeatOf(c);
		string seats;
		foreach (int i, string n : t.m_aSeatNames)
		{
			if (i > 0)
				seats += ", ";

			seats += string.Format("%1. %2", i + 1, n);
		}

		string board;
		string line;
		if (!t.m_bStarted)
		{
			if (seat == 0)
				line = string.Format("Waiting for players (%1 of %2-%3). Add bots to play alone, then press Start.", t.m_aSeats.Count(), t.m_Rules.MinSeats(), t.m_Rules.MaxSeats());
			else
				line = "Waiting for the host to start the match.";
		}
		else
		{
			int turn = t.m_Rules.GetTurn();
			board = t.m_Rules.View(t.PlaySeat(c));
			int w = t.m_Rules.GetWinner();
			if (w == -2)
				line = "The match is a draw.";
			else if (w >= 0 && t.m_aSeats.IsIndexValid(w) && t.m_aSeats[w] == c)
				line = "You won!";
			else if (w >= 0)
				line = "You lost.";
			else if (turn < 0)
				line = "Every player may act now.";
			else if (t.m_aSeats.IsIndexValid(turn) && t.m_aSeats[turn] == c)
				line = "Your turn.";
			else
				line = "Waiting for the other player.";
		}

		string name = t.m_sName;
		if (name.IsEmpty())
			name = t.m_Rules.GetTitle() + " #" + t.m_iId;

		// Seat played, when this computer holds more than one.
		string playing;
		array<int> own = {};
		t.SeatsOf(c, own);
		if (own.Count() > 1)
			playing = string.Format("Playing seat %1 of %2 held here (%3)", t.PlaySeat(c) + 1, own.Count(), t.m_aSeatNames[t.PlaySeat(c)]);

		array<string> res = {
			t.m_iId.ToString(),
			t.m_sGame,
			name,
			ROOT_AE3_Util.BoolStr(t.m_bStarted),
			ROOT_AE3_Util.BoolStr(seat == 0),
			seats,
			board,
			line,
			t.m_Rules.GetHelp(),
			status,
			playing,
			ROOT_AE3_Util.BoolStr(ROOT_AE3_SettingsManager.Get().m_bGamesDebugSeats)
		};
		ROOT_AE3_GuiRouter.Reply(s, rid, true, "game_view", res);
	}
}
