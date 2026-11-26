//! Rules of one networked game. Server only. Subclasses keep the full state; View hides other seats' secrets.
class ROOT_AE3_GameRules
{
	protected static ref array<string> s_aGames;

	int m_iSeats;
	int m_iTurn;
	int m_iWinner = -1;		//!< -1 running, -2 draw, else winning seat
	string m_sStatus;

	//------------------------------------------------------------------------------------------------
	static array<string> Games()
	{
		if (!s_aGames)
			s_aGames = {"chess", "checkers", "uno", "ludo", "snakesladders", "blackjack", "rps", "battleship"};

		return s_aGames;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_GameRules Create(string id)
	{
		switch (id)
		{
			case "chess": return new ROOT_AE3_GameChess();
			case "checkers": return new ROOT_AE3_GameCheckers();
			case "uno": return new ROOT_AE3_GameUno();
			case "ludo": return new ROOT_AE3_GameLudo();
			case "snakesladders": return new ROOT_AE3_GameSnakesLadders();
			case "blackjack": return new ROOT_AE3_GameBlackjack();
			case "rps": return new ROOT_AE3_GameRps();
			case "battleship": return new ROOT_AE3_GameBattleship();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	string GetId()
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	string GetTitle()
	{
		return GetId();
	}

	//------------------------------------------------------------------------------------------------
	int MinSeats()
	{
		return 2;
	}

	//------------------------------------------------------------------------------------------------
	int MaxSeats()
	{
		return 2;
	}

	//------------------------------------------------------------------------------------------------
	string GetHelp()
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! starter: seat that moves first. pref: per-seat side wishes (chess).
	void Start(int seats, int starter)
	{
		m_iSeats = seats;
		m_iTurn = starter;
		m_iWinner = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Seat to move, -1 when every seat may act.
	int GetTurn()
	{
		if (m_iWinner != -1)
			return -1;

		return m_iTurn;
	}

	//------------------------------------------------------------------------------------------------
	int GetWinner()
	{
		return m_iWinner;
	}

	//------------------------------------------------------------------------------------------------
	bool Apply(int seat, string move, out string status)
	{
		status = "Unknown command.";
		return false;
	}

	//------------------------------------------------------------------------------------------------
	string View(int seat)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! State for the graphical board as key=value pairs joined by ';' (lists joined by ','), seen from seat.
	string Gui(int seat)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Numbers joined by ','.
	static string Ints(notnull array<int> a)
	{
		string s;
		foreach (int i, int v : a)
		{
			if (i > 0)
				s += ",";

			s += v.ToString();
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! Moves a bot in that seat would make now, best first. Empty when the seat has nothing to do.
	void BotMoves(int seat, notnull array<string> moves)
	{
	}

	//------------------------------------------------------------------------------------------------
	static void BotShuffle(notnull array<string> a)
	{
		for (int i = a.Count() - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			string t = a[i];
			a[i] = a[j];
			a[j] = t;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Appends cands to moves, highest score first.
	static void BotRanked(notnull array<string> cands, notnull array<float> scores, notnull array<string> moves)
	{
		array<int> used = {};
		for (int n = 0; n < cands.Count(); n++)
		{
			int best = -1;
			for (int i = 0; i < cands.Count(); i++)
			{
				if (used.Contains(i))
					continue;

				if (best < 0 || scores[i] > scores[best])
					best = i;
			}

			used.Insert(best);
			moves.Insert(cands[best]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The seat to move ran out of time: two-seat games are lost, others skip the turn.
	void OnTimeout()
	{
		if (m_iTurn < 0)
			return;

		if (m_iSeats == 2)
		{
			m_iWinner = 1 - m_iTurn;
			m_sStatus = "A player ran out of time.";
			return;
		}

		NextTurn();
		m_sStatus = "A player ran out of time; the turn passed.";
	}

	//------------------------------------------------------------------------------------------------
	//! A seat left during play.
	void OnSeatLeft(int seat)
	{
		if (m_iWinner != -1)
			return;

		if (m_iSeats == 2)
		{
			m_iWinner = 1 - seat;
			m_sStatus = "A player left the table.";
		}
		else if (m_iTurn == seat)
		{
			NextTurn();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void NextTurn()
	{
		if (m_iSeats > 0)
			m_iTurn = (m_iTurn + 1) % m_iSeats;
	}

	//------------------------------------------------------------------------------------------------
	//! Lowercase words of a move.
	static array<string> Words(string move)
	{
		move.ToLower();
		return ROOT_AE3_Util.Words(move);
	}

	//------------------------------------------------------------------------------------------------
	//! Board square "e4" to index (file + rank * 8), -1 when invalid. size = board edge.
	static int Square(string s, int size = 8)
	{
		if (s.Length() < 2 || s.Length() > 3)
			return -1;

		int file = s.ToAscii(0) - 97;
		string rankStr = s.Substring(1, s.Length() - 1);
		if (!ROOT_AE3_Util.IsInt(rankStr))
			return -1;

		int rank = rankStr.ToInt() - 1;
		if (file < 0 || file >= size || rank < 0 || rank >= size)
			return -1;

		return rank * size + file;
	}

	//------------------------------------------------------------------------------------------------
	static string SquareName(int idx, int size = 8)
	{
		int file = idx % size;
		int rank = idx / size;
		return (97 + file).AsciiToString() + (rank + 1).ToString();
	}
}
