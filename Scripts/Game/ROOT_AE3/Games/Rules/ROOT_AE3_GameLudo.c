//! Ludo for 2-4 seats, four tokens each. A six leaves the yard and rolls again; landing on an opponent
//! sends it back to its yard. Token progress: -1 yard, 0-51 track, 52-56 home column, 57 home.
class ROOT_AE3_GameLudo : ROOT_AE3_GameRules
{
	protected ref array<int> m_aTokens = {};	//!< seat * 4 + token
	protected int m_iRoll;					//!< 0 = not rolled yet
	protected int m_iSixes;
	protected int m_iShown;					//!< last die value thrown

	//------------------------------------------------------------------------------------------------
	override string GetId()
	{
		return "ludo";
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Ludo";
	}

	//------------------------------------------------------------------------------------------------
	override int MaxSeats()
	{
		return 4;
	}

	//------------------------------------------------------------------------------------------------
	override string GetHelp()
	{
		return "Type roll, then move <token 1-4>. A 6 leaves the yard and rolls again. Bring all four tokens home.";
	}

	//------------------------------------------------------------------------------------------------
	override void Start(int seats, int starter)
	{
		super.Start(seats, starter);
		m_aTokens.Clear();
		for (int i = 0; i < seats * 4; i++)
		{
			m_aTokens.Insert(-1);
		}

		m_iRoll = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Absolute track square of a token, -1 when not on the shared track.
	protected int Absolute(int seat, int progress)
	{
		if (progress < 0 || progress > 51)
			return -1;

		return (progress + seat * 13) % 52;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanMove(int seat, int token)
	{
		int p = m_aTokens[seat * 4 + token];
		if (p == -1)
			return m_iRoll == 6;

		if (p >= 57)
			return false;

		return p + m_iRoll <= 57;
	}

	//------------------------------------------------------------------------------------------------
	protected bool AnyMove(int seat)
	{
		for (int t = 0; t < 4; t++)
		{
			if (CanMove(seat, t))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void EndTurn()
	{
		bool again = m_iRoll == 6 && m_iSixes < 3;
		m_iRoll = 0;
		if (!again)
		{
			m_iSixes = 0;
			NextTurn();
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool Apply(int seat, string move, out string status)
	{
		if (m_iWinner != -1)
		{
			status = "This match is over.";
			return false;
		}

		if (seat != m_iTurn)
		{
			status = "It is not your turn.";
			return false;
		}

		array<string> w = Words(move);
		if (w.IsEmpty())
		{
			status = "Type roll to throw the die.";
			return false;
		}

		if (w[0] == "roll")
		{
			if (m_iRoll > 0)
			{
				status = "You have rolled - move a token.";
				return false;
			}

			m_iRoll = Math.RandomIntInclusive(1, 6);
			m_iShown = m_iRoll;
			if (m_iRoll == 6)
				m_iSixes++;

			status = string.Format("Rolled %1.", m_iRoll);
			if (!AnyMove(seat))
			{
				status += " Nothing that roll can move.";
				EndTurn();
			}

			m_sStatus = status;
			return true;
		}

		if (w[0] != "move" || w.Count() < 2)
		{
			status = "Type roll, then move <token>.";
			return false;
		}

		if (m_iRoll == 0)
		{
			status = "Roll before moving a token.";
			return false;
		}

		int token = w[1].ToInt() - 1;
		if (token < 0 || token > 3 || !CanMove(seat, token))
		{
			status = "That token cannot make that move.";
			return false;
		}

		int idx = seat * 4 + token;
		if (m_aTokens[idx] == -1)
			m_aTokens[idx] = 0;
		else
			m_aTokens[idx] = m_aTokens[idx] + m_iRoll;

		status = "Token moved.";
		int abs = Absolute(seat, m_aTokens[idx]);
		if (abs >= 0)
		{
			for (int s = 0; s < m_iSeats; s++)
			{
				if (s == seat)
					continue;

				for (int t = 0; t < 4; t++)
				{
					if (Absolute(s, m_aTokens[s * 4 + t]) == abs)
					{
						m_aTokens[s * 4 + t] = -1;
						status = "A token was sent back to its yard.";
					}
				}
			}
		}

		if (m_aTokens[idx] == 57)
			status = "A token reached home.";

		bool all = true;
		for (int t = 0; t < 4; t++)
		{
			if (m_aTokens[seat * 4 + t] != 57)
				all = false;
		}

		if (all)
		{
			m_iWinner = seat;
			status = "All four tokens are home - the game is won.";
			m_sStatus = status;
			return true;
		}

		EndTurn();
		m_sStatus = status;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string Describe(int p)
	{
		if (p == -1)
			return "yard";

		if (p >= 57)
			return "HOME";

		if (p >= 52)
			return "home column " + (p - 51).ToString();

		return "track " + p.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! Roll, then the token that reaches home, captures, leaves the yard or is furthest along.
	override void BotMoves(int seat, notnull array<string> moves)
	{
		if (m_iWinner != -1 || seat != m_iTurn)
			return;

		if (m_iRoll == 0)
		{
			moves.Insert("roll");
			return;
		}

		array<string> cands = {};
		array<float> scores = {};
		for (int t = 0; t < 4; t++)
		{
			if (!CanMove(seat, t))
				continue;

			int p = m_aTokens[seat * 4 + t];
			int np = p + m_iRoll;
			float score = p + Math.RandomFloat(0, 1);
			if (p == -1)
			{
				np = 0;
				score = 100;
			}

			if (np == 57)
				score = 200;

			int abs = Absolute(seat, np);
			for (int s = 0; s < m_iSeats && abs >= 0; s++)
			{
				if (s == seat)
					continue;

				for (int o = 0; o < 4; o++)
				{
					if (Absolute(s, m_aTokens[s * 4 + o]) == abs)
						score += 150;
				}
			}

			cands.Insert("move " + (t + 1).ToString());
			scores.Insert(score);
		}

		BotRanked(cands, scores, moves);
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! tokens (seat * 4 + token progress), roll (0 not rolled), shown (last die), can (per own token 1 when it may move).
	override string Gui(int seat)
	{
		array<int> can = {};
		for (int t = 0; t < 4; t++)
		{
			int c;
			if (m_iWinner == -1 && seat == m_iTurn && m_iRoll > 0 && CanMove(seat, t))
				c = 1;

			can.Insert(c);
		}

		return "tokens=" + Ints(m_aTokens) + ";roll=" + m_iRoll.ToString() + ";shown=" + m_iShown.ToString() + ";can=" + Ints(can);
	}

	//------------------------------------------------------------------------------------------------
	override string View(int seat)
	{
		string v;
		for (int s = 0; s < m_iSeats; s++)
		{
			string me;
			if (s == seat)
				me = " (you)";

			v += string.Format("Seat %1%2 (starts at square %3):\n", s + 1, me, s * 13);
			for (int t = 0; t < 4; t++)
			{
				v += string.Format("  token %1: %2\n", t + 1, Describe(m_aTokens[s * 4 + t]));
			}
		}

		if (m_iRoll > 0)
			v += "Die: " + m_iRoll.ToString();
		else
			v += "Die: not rolled";

		return v;
	}
}
