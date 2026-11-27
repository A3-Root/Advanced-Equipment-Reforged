//! Snakes and ladders on 100 squares for 2-4 seats. Exact roll needed to reach 100.
class ROOT_AER_GameSnakesLadders : ROOT_AER_GameRules
{
	protected ref array<int> m_aPos = {};
	protected ref map<int, int> m_mJumps = new map<int, int>();
	protected int m_iLastRoll;

	//------------------------------------------------------------------------------------------------
	override string GetId()
	{
		return "snakesladders";
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Snakes and Ladders";
	}

	//------------------------------------------------------------------------------------------------
	override int MaxSeats()
	{
		return 4;
	}

	//------------------------------------------------------------------------------------------------
	override string GetHelp()
	{
		return "Type roll to throw the die. Ladders lift you, snakes drop you. Land exactly on 100 to win.";
	}

	//------------------------------------------------------------------------------------------------
	override void Start(int seats, int starter)
	{
		super.Start(seats, starter);
		m_aPos.Clear();
		for (int i = 0; i < seats; i++)
		{
			m_aPos.Insert(1);
		}

		array<int> jumps = {4, 14, 9, 31, 20, 38, 28, 84, 40, 59, 51, 67, 63, 81, 71, 91, 17, 7, 54, 34, 62, 19, 64, 60, 87, 24, 93, 73, 95, 75, 99, 78};
		for (int i = 0; i + 1 < jumps.Count(); i += 2)
		{
			m_mJumps.Set(jumps[i], jumps[i + 1]);
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
		if (w.IsEmpty() || w[0] != "roll")
		{
			status = "Type roll to throw the die.";
			return false;
		}

		m_iLastRoll = Math.RandomIntInclusive(1, 6);
		int target = m_aPos[seat] + m_iLastRoll;
		status = string.Format("Rolled %1.", m_iLastRoll);
		if (target > 100)
		{
			status += " That roll would pass the last square, so nothing moves.";
		}
		else
		{
			int jump;
			if (m_mJumps.Find(target, jump))
			{
				if (jump > target)
					status += string.Format(" Up a ladder to %1.", jump);
				else
					status += string.Format(" Down a snake to %1.", jump);

				target = jump;
			}

			m_aPos[seat] = target;
			if (target == 100)
			{
				m_iWinner = seat;
				status += " Home - the game is won.";
				m_sStatus = status;
				return true;
			}
		}

		NextTurn();
		m_sStatus = status;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void BotMoves(int seat, notnull array<string> moves)
	{
		if (m_iWinner == -1 && seat == m_iTurn)
			moves.Insert("roll");
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! pos (square per seat), roll (last die), jumps (from, to pairs).
	override string Gui(int seat)
	{
		array<int> jumps = {};
		foreach (int from, int to : m_mJumps)
		{
			jumps.Insert(from);
			jumps.Insert(to);
		}

		return "pos=" + Ints(m_aPos) + ";roll=" + m_iLastRoll.ToString() + ";jumps=" + Ints(jumps);
	}

	//------------------------------------------------------------------------------------------------
	override string View(int seat)
	{
		string v = "Positions:\n";
		for (int i = 0; i < m_iSeats; i++)
		{
			string me;
			if (i == seat)
				me = " (you)";

			v += string.Format("  Seat %1%2: square %3\n", i + 1, me, m_aPos[i]);
		}

		v += "Ladders: 4>14 9>31 20>38 28>84 40>59 51>67 63>81 71>91\n";
		v += "Snakes: 17>7 54>34 62>19 64>60 87>24 93>73 95>75 99>78";
		if (m_iLastRoll > 0)
			v += "\nLast roll: " + m_iLastRoll.ToString();

		return v;
	}
}
