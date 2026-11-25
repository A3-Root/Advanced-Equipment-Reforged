//! Blackjack for 1-8 seats against the dealer. Each round: deal, every seat hits or stands in order,
//! dealer draws to 17. A round won scores one point; first seat to 5 points wins the match.
class ROOT_AE3_GameBlackjack : ROOT_AE3_GameRules
{
	protected ref array<string> m_aDeck = {};
	protected ref array<ref array<string>> m_aHands = {};
	protected ref array<string> m_aDealer = {};
	protected ref array<bool> m_aDone = {};
	protected ref array<int> m_aScore = {};
	protected bool m_bInRound;
	protected string m_sRoundResult;

	//------------------------------------------------------------------------------------------------
	override string GetId()
	{
		return "blackjack";
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Blackjack";
	}

	//------------------------------------------------------------------------------------------------
	override int MinSeats()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override int MaxSeats()
	{
		return 8;
	}

	//------------------------------------------------------------------------------------------------
	override string GetHelp()
	{
		return "deal starts a round, then hit or stand on your turn. Beat the dealer without passing 21. First to 5 rounds wins.";
	}

	//------------------------------------------------------------------------------------------------
	override void Start(int seats, int starter)
	{
		super.Start(seats, starter);
		m_aScore.Clear();
		m_aHands.Clear();
		m_aDone.Clear();
		for (int s = 0; s < seats; s++)
		{
			m_aScore.Insert(0);
			array<string> h = {};
			m_aHands.Insert(h);
			m_aDone.Insert(false);
		}

		m_bInRound = false;
	}

	//------------------------------------------------------------------------------------------------
	override int GetTurn()
	{
		if (m_iWinner != -1 || !m_bInRound)
			return -1;

		return m_iTurn;
	}

	//------------------------------------------------------------------------------------------------
	protected void NewDeck()
	{
		m_aDeck.Clear();
		array<string> ranks = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
		array<string> suits = {"c", "d", "h", "s"};
		foreach (string s : suits)
		{
			foreach (string r : ranks)
			{
				m_aDeck.Insert(r + s);
			}
		}

		for (int i = m_aDeck.Count() - 1; i > 0; i--)
		{
			int j = Math.RandomInt(0, i + 1);
			string t = m_aDeck[i];
			m_aDeck[i] = m_aDeck[j];
			m_aDeck[j] = t;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string Card()
	{
		if (m_aDeck.Count() < 10)
			NewDeck();

		string c = m_aDeck[m_aDeck.Count() - 1];
		m_aDeck.Remove(m_aDeck.Count() - 1);
		return c;
	}

	//------------------------------------------------------------------------------------------------
	static int Value(array<string> hand)
	{
		int total;
		int aces;
		foreach (string c : hand)
		{
			string r = c.Substring(0, c.Length() - 1);
			if (r == "A")
			{
				aces++;
				total += 11;
			}
			else if (r == "J" || r == "Q" || r == "K")
			{
				total += 10;
			}
			else
			{
				total += r.ToInt();
			}
		}

		while (total > 21 && aces > 0)
		{
			total -= 10;
			aces--;
		}

		return total;
	}

	//------------------------------------------------------------------------------------------------
	protected void AdvanceOrResolve()
	{
		for (int i = 1; i <= m_iSeats; i++)
		{
			int s = (m_iTurn + i) % m_iSeats;
			if (!m_aDone[s])
			{
				m_iTurn = s;
				return;
			}
		}

		Resolve();
	}

	//------------------------------------------------------------------------------------------------
	protected void Resolve()
	{
		while (Value(m_aDealer) < 17)
		{
			m_aDealer.Insert(Card());
		}

		int dealer = Value(m_aDealer);
		m_sRoundResult = string.Format("Dealer has %1.", dealer);
		for (int s = 0; s < m_iSeats; s++)
		{
			int v = Value(m_aHands[s]);
			string r = "loses";
			if (v <= 21 && (dealer > 21 || v > dealer))
			{
				r = "wins";
				m_aScore[s] = m_aScore[s] + 1;
				if (m_aScore[s] >= 5 && m_iWinner == -1)
					m_iWinner = s;
			}
			else if (v <= 21 && v == dealer)
			{
				r = "pushes";
			}

			m_sRoundResult += string.Format(" Seat %1 %2 (%3).", s + 1, r, v);
		}

		m_bInRound = false;
		m_sStatus = "The dealer has played and the round is scored.";
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
		if (w.IsEmpty())
		{
			status = "Type hit or stand.";
			return false;
		}

		if (w[0] == "deal")
		{
			if (m_bInRound)
			{
				status = "This round is still being played.";
				return false;
			}

			m_aDealer.Clear();
			m_aDealer.Insert(Card());
			m_aDealer.Insert(Card());
			for (int s = 0; s < m_iSeats; s++)
			{
				m_aHands[s].Clear();
				m_aHands[s].Insert(Card());
				m_aHands[s].Insert(Card());
				m_aDone[s] = Value(m_aHands[s]) == 21;
			}

			m_bInRound = true;
			m_sRoundResult = "";
			m_iTurn = m_iSeats - 1;
			status = "Cards dealt.";
			AdvanceOrResolve();
			m_sStatus = status;
			return true;
		}

		if (!m_bInRound)
		{
			status = "Type deal to start the next round.";
			return false;
		}

		if (seat != m_iTurn)
		{
			status = "It is not your turn.";
			return false;
		}

		if (w[0] == "hit")
		{
			m_aHands[seat].Insert(Card());
			int v = Value(m_aHands[seat]);
			status = "Card taken.";
			if (v > 21)
			{
				status = "Bust.";
				m_aDone[seat] = true;
				AdvanceOrResolve();
			}
			else if (v == 21)
			{
				status = "Twenty one.";
				m_aDone[seat] = true;
				AdvanceOrResolve();
			}

			m_sStatus = status;
			return true;
		}

		if (w[0] == "stand")
		{
			m_aDone[seat] = true;
			status = "Standing.";
			AdvanceOrResolve();
			m_sStatus = status;
			return true;
		}

		status = "Type hit or stand.";
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		if (!m_bInRound || m_iTurn < 0)
			return;

		m_aDone[m_iTurn] = true;
		m_sStatus = "A player ran out of time and stands.";
		AdvanceOrResolve();
	}

	//------------------------------------------------------------------------------------------------
	override void OnSeatLeft(int seat)
	{
		if (seat < m_aDone.Count() && m_bInRound)
		{
			m_aDone[seat] = true;
			AdvanceOrResolve();
		}
	}

	//------------------------------------------------------------------------------------------------
	override string View(int seat)
	{
		string dealer;
		if (m_bInRound && !m_aDealer.IsEmpty())
			dealer = m_aDealer[0] + " ??";
		else if (!m_aDealer.IsEmpty())
			dealer = ROOT_AE3_Util.JoinFrom(m_aDealer, 0) + " (" + Value(m_aDealer).ToString() + ")";
		else
			dealer = "-";

		string v = "Dealer: " + dealer + "\n";
		for (int s = 0; s < m_iSeats; s++)
		{
			string me;
			if (s == seat)
				me = " (you)";

			string hand = "-";
			if (!m_aHands[s].IsEmpty())
				hand = ROOT_AE3_Util.JoinFrom(m_aHands[s], 0) + " (" + Value(m_aHands[s]).ToString() + ")";

			v += string.Format("Seat %1%2: %3   points %4\n", s + 1, me, hand, m_aScore[s]);
		}

		if (!m_sRoundResult.IsEmpty())
			v += m_sRoundResult;
		else if (!m_bInRound)
			v += "Type deal to start the next round.";

		return v;
	}
}
