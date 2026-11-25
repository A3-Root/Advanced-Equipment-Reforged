//! Rock paper scissors, first to three rounds. Both seats throw at any time.
class ROOT_AE3_GameRps : ROOT_AE3_GameRules
{
	protected ref array<string> m_aThrows = {"", ""};
	protected ref array<string> m_aLast = {"", ""};
	protected ref array<int> m_aScores = {0, 0};
	protected int m_iRound = 1;

	//------------------------------------------------------------------------------------------------
	override string GetId()
	{
		return "rps";
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Rock Paper Scissors";
	}

	//------------------------------------------------------------------------------------------------
	override string GetHelp()
	{
		return "Throw rock, paper or scissors (r, p, s). First to 3 rounds wins.";
	}

	//------------------------------------------------------------------------------------------------
	override int GetTurn()
	{
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Normalize(string w)
	{
		if (w == "r" || w == "rock")
			return "rock";

		if (w == "p" || w == "paper")
			return "paper";

		if (w == "s" || w == "scissors")
			return "scissors";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Beats(string a, string b)
	{
		return (a == "rock" && b == "scissors") || (a == "paper" && b == "rock") || (a == "scissors" && b == "paper");
	}

	//------------------------------------------------------------------------------------------------
	override bool Apply(int seat, string move, out string status)
	{
		if (m_iWinner != -1)
		{
			status = "This match is over.";
			return false;
		}

		array<string> w = Words(move);
		string t;
		if (!w.IsEmpty())
			t = Normalize(w[0]);

		if (t.IsEmpty())
		{
			status = "Throw rock, paper or scissors.";
			return false;
		}

		if (!m_aThrows[seat].IsEmpty())
		{
			status = "You have already thrown this round.";
			return false;
		}

		m_aThrows[seat] = t;
		if (m_aThrows[1 - seat].IsEmpty())
		{
			status = "Thrown - waiting for the other player.";
			m_sStatus = status;
			return true;
		}

		m_aLast[0] = m_aThrows[0];
		m_aLast[1] = m_aThrows[1];
		if (m_aThrows[0] == m_aThrows[1])
		{
			status = "The round is a draw.";
		}
		else if (Beats(m_aThrows[0], m_aThrows[1]))
		{
			m_aScores[0] = m_aScores[0] + 1;
			status = "The first seat takes the round.";
		}
		else
		{
			m_aScores[1] = m_aScores[1] + 1;
			status = "The second seat takes the round.";
		}

		m_aThrows[0] = "";
		m_aThrows[1] = "";
		m_iRound++;
		for (int i = 0; i < 2; i++)
		{
			if (m_aScores[i] >= 3)
				m_iWinner = i;
		}

		m_sStatus = status;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		for (int i = 0; i < 2; i++)
		{
			if (m_aThrows[i].IsEmpty())
			{
				m_iWinner = 1 - i;
				m_sStatus = "A player ran out of time.";
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override string View(int seat)
	{
		string mine = m_aThrows[seat];
		if (mine.IsEmpty())
			mine = "-";

		string theirs = "not yet";
		if (!m_aThrows[1 - seat].IsEmpty())
			theirs = "thrown";

		string v = string.Format("Round %1   Score: you %2 - %3 opponent\n", m_iRound, m_aScores[seat], m_aScores[1 - seat]);
		v += string.Format("Your throw: %1   Opponent: %2\n", mine, theirs);
		if (!m_aLast[0].IsEmpty())
			v += string.Format("Last round: you %1, opponent %2", m_aLast[seat], m_aLast[1 - seat]);

		return v;
	}
}
