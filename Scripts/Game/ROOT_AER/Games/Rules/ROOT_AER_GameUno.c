//! Uno for 2-8 seats. Cards: colour r g b y with 0-9, S skip, R reverse, D draw two; wild W and W4.
//! Play by name (play r5, play gS, play W b, play W4 y) or draw.
class ROOT_AER_GameUno : ROOT_AER_GameRules
{
	protected ref array<string> m_aDeck = {};
	protected ref array<string> m_aDiscard = {};
	protected ref array<ref array<string>> m_aHands = {};
	protected string m_sColour;
	protected int m_iDir = 1;

	//------------------------------------------------------------------------------------------------
	override string GetId()
	{
		return "uno";
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Uno";
	}

	//------------------------------------------------------------------------------------------------
	override int MaxSeats()
	{
		return 8;
	}

	//------------------------------------------------------------------------------------------------
	override string GetHelp()
	{
		return "play <card> (r5, gS skip, bR reverse, yD draw two, W <colour>, W4 <colour>) or draw. First to empty the hand wins.";
	}

	//------------------------------------------------------------------------------------------------
	override void Start(int seats, int starter)
	{
		super.Start(seats, starter);
		m_aDeck.Clear();
		array<string> colours = {"r", "g", "b", "y"};
		foreach (string c : colours)
		{
			m_aDeck.Insert(c + "0");
			for (int n = 1; n <= 9; n++)
			{
				m_aDeck.Insert(c + n.ToString());
				m_aDeck.Insert(c + n.ToString());
			}

			for (int k = 0; k < 2; k++)
			{
				m_aDeck.Insert(c + "S");
				m_aDeck.Insert(c + "R");
				m_aDeck.Insert(c + "D");
			}
		}

		for (int k = 0; k < 4; k++)
		{
			m_aDeck.Insert("W");
			m_aDeck.Insert("W4");
		}

		Shuffle(m_aDeck);
		m_aHands.Clear();
		for (int s = 0; s < seats; s++)
		{
			array<string> h = {};
			m_aHands.Insert(h);
			for (int i = 0; i < 7; i++)
			{
				h.Insert(DrawCard());
			}
		}

		string first = DrawCard();
		while (first.StartsWith("W"))
		{
			m_aDeck.InsertAt(first, 0);
			first = DrawCard();
		}

		m_aDiscard.Insert(first);
		m_sColour = first.Substring(0, 1);
		m_iDir = 1;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Shuffle(array<string> a)
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
	protected string DrawCard()
	{
		if (m_aDeck.IsEmpty())
		{
			string top = m_aDiscard[m_aDiscard.Count() - 1];
			m_aDiscard.Remove(m_aDiscard.Count() - 1);
			m_aDeck.Copy(m_aDiscard);
			m_aDiscard.Clear();
			m_aDiscard.Insert(top);
			Shuffle(m_aDeck);
		}

		if (m_aDeck.IsEmpty())
			return "W";

		string c = m_aDeck[m_aDeck.Count() - 1];
		m_aDeck.Remove(m_aDeck.Count() - 1);
		return c;
	}

	//------------------------------------------------------------------------------------------------
	protected string Top()
	{
		return m_aDiscard[m_aDiscard.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	protected bool Playable(string card)
	{
		if (card.StartsWith("W"))
			return true;

		string top = Top();
		string value = card.Substring(1, card.Length() - 1);
		string topValue = top.Substring(1, top.Length() - 1);
		return card.Substring(0, 1) == m_sColour || (!top.StartsWith("W") && value == topValue);
	}

	//------------------------------------------------------------------------------------------------
	protected int Next(int seat)
	{
		return (seat + m_iDir + m_iSeats) % m_iSeats;
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

		array<string> w = ROOT_AER_Util.Words(move);
		if (w.IsEmpty())
		{
			status = "Type play followed by a card, or draw.";
			return false;
		}

		string verb = w[0];
		verb.ToLower();
		array<string> hand = m_aHands[seat];
		if (verb == "draw")
		{
			hand.Insert(DrawCard());
			m_iTurn = Next(seat);
			status = "Card drawn.";
			m_sStatus = status;
			return true;
		}

		if (verb != "play" || w.Count() < 2)
		{
			status = "Type play followed by a card, or draw.";
			return false;
		}

		string card = w[1];
		if (card.Length() >= 2 && !card.StartsWith("w") && !card.StartsWith("W"))
		{
			string colour = card.Substring(0, 1);
			colour.ToLower();
			string value = card.Substring(1, card.Length() - 1);
			value.ToUpper();
			card = colour + value;
		}
		else
		{
			card.ToUpper();
		}

		int idx = hand.Find(card);
		if (idx < 0)
		{
			status = "You do not hold that card.";
			return false;
		}

		if (!Playable(card))
		{
			status = "That card does not match the colour or the value in play.";
			return false;
		}

		string newColour = card.Substring(0, 1);
		if (card.StartsWith("W"))
		{
			if (w.Count() < 3)
			{
				status = "A wild needs a colour: play W r for red.";
				return false;
			}

			newColour = w[2].Substring(0, 1);
			newColour.ToLower();
			if (newColour != "r" && newColour != "g" && newColour != "b" && newColour != "y")
			{
				status = "A wild needs a colour: play W r for red.";
				return false;
			}
		}

		hand.Remove(idx);
		m_aDiscard.Insert(card);
		m_sColour = newColour;
		status = "Card played.";
		if (hand.IsEmpty())
		{
			m_iWinner = seat;
			status = "Last card played - the game is won.";
			m_sStatus = status;
			return true;
		}

		string kind = card.Substring(card.Length() - 1, 1);
		int next = Next(seat);
		if (card == "W4")
		{
			for (int i = 0; i < 4; i++)
			{
				m_aHands[next].Insert(DrawCard());
			}

			next = Next(next);
			status = "The next seat draws and is skipped.";
		}
		else if (!card.StartsWith("W") && kind == "D")
		{
			m_aHands[next].Insert(DrawCard());
			m_aHands[next].Insert(DrawCard());
			next = Next(next);
			status = "The next seat draws and is skipped.";
		}
		else if (!card.StartsWith("W") && kind == "S")
		{
			next = Next(next);
			status = "The next seat is skipped.";
		}
		else if (!card.StartsWith("W") && kind == "R")
		{
			m_iDir = -m_iDir;
			status = "The order of play is reversed.";
			if (m_iSeats == 2)
				next = seat;
			else
				next = Next(seat);
		}

		m_iTurn = next;
		m_sStatus = status;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		if (m_iTurn < 0)
			return;

		m_aHands[m_iTurn].Insert(DrawCard());
		m_iTurn = Next(m_iTurn);
		m_sStatus = "A player ran out of time and drew a card.";
	}

	//------------------------------------------------------------------------------------------------
	//! Matching colour or value cards, then wilds in the colour held most, then draw.
	override void BotMoves(int seat, notnull array<string> moves)
	{
		if (m_iWinner != -1 || seat != m_iTurn)
			return;

		array<string> colours = {"r", "g", "b", "y"};
		array<int> counts = {0, 0, 0, 0};
		array<string> hand = m_aHands[seat];
		foreach (string card : hand)
		{
			int ci = colours.Find(card.Substring(0, 1));
			if (ci >= 0)
				counts[ci] = counts[ci] + 1;
		}

		int best = 0;
		for (int i = 1; i < 4; i++)
		{
			if (counts[i] > counts[best])
				best = i;
		}

		array<string> plain = {};
		array<string> wild = {};
		foreach (string card : hand)
		{
			if (!Playable(card))
				continue;

			if (card.StartsWith("W"))
				wild.Insert("play " + card + " " + colours[best]);
			else
				plain.Insert("play " + card);
		}

		BotShuffle(plain);
		moves.InsertAll(plain);
		moves.InsertAll(wild);
		moves.Insert("draw");
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! hand, ok (per card 1 when playable now), top, colour, counts (cards per seat), dir, deck.
	override string Gui(int seat)
	{
		array<string> hand = m_aHands[seat];
		array<int> ok = {};
		foreach (string card : hand)
		{
			int can;
			if (m_iWinner == -1 && seat == m_iTurn && Playable(card))
				can = 1;

			ok.Insert(can);
		}

		array<int> counts = {};
		foreach (array<string> h : m_aHands)
		{
			counts.Insert(h.Count());
		}

		string g = "hand=" + string.Join(",", hand, true) + ";ok=" + Ints(ok) + ";top=" + Top() + ";colour=" + m_sColour;
		return g + ";counts=" + Ints(counts) + ";dir=" + m_iDir.ToString() + ";deck=" + m_aDeck.Count().ToString();
	}

	//------------------------------------------------------------------------------------------------
	override string View(int seat)
	{
		string colourName = "red";
		switch (m_sColour)
		{
			case "g": colourName = "green"; break;
			case "b": colourName = "blue"; break;
			case "y": colourName = "yellow"; break;
		}

		string v = string.Format("Top card: %1   Colour in play: %2\n", Top(), colourName);
		v += "Your hand: " + ROOT_AER_Util.JoinFrom(m_aHands[seat], 0) + "\n";
		for (int s = 0; s < m_iSeats; s++)
		{
			string me;
			if (s == seat)
				me = " (you)";

			v += string.Format("  Seat %1%2: %3 cards\n", s + 1, me, m_aHands[s].Count());
		}

		if (m_iDir > 0)
			v += "Direction: forward";
		else
			v += "Direction: reversed";

		return v;
	}
}
