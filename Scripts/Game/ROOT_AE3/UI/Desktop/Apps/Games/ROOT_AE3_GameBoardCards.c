//! Uno table: opponents on top, draw pile and discard pile in the middle, own hand at the bottom.
//! Playable cards are raised; a wild asks for the colour.
class ROOT_AE3_GameBoardUno : ROOT_AE3_GameBoard
{
	protected int m_iWild = -1;		//!< hand index of the wild waiting for a colour

	//------------------------------------------------------------------------------------------------
	protected override void OnNewState()
	{
		m_iWild = -1;
	}

	//------------------------------------------------------------------------------------------------
	static int CardColor(string code)
	{
		string c = code.Substring(0, 1);
		switch (c)
		{
			case "r": return 0xFFD7301F;
			case "g": return 0xFF3C9A2E;
			case "b": return 0xFF1C5FC4;
			case "y": return 0xFFE8B90C;
		}

		return 0xFF1A1A1A;
	}

	//------------------------------------------------------------------------------------------------
	static string ColourName(string c)
	{
		switch (c)
		{
			case "r": return "Red";
			case "g": return "Green";
			case "b": return "Blue";
			case "y": return "Yellow";
		}

		return c;
	}

	//------------------------------------------------------------------------------------------------
	static string Label(string code)
	{
		if (code == "W")
			return "WILD";

		if (code == "W4")
			return "+4";

		string v = code.Substring(1, code.Length() - 1);
		switch (v)
		{
			case "S": return "SKIP";
			case "R": return "REV";
			case "D": return "+2";
		}

		return v;
	}

	//------------------------------------------------------------------------------------------------
	//! Face of a card; empty code draws the back.
	protected void Card(float x, float y, float w, float h, string code, bool dim = false)
	{
		float r = w * 0.12;
		m_Paint.Round(x + 3, y + 5, w, h, r, 0x66000000);
		m_Paint.Round(x, y, w, h, r, 0xFFFFFFFF);
		if (code.IsEmpty())
		{
			m_Paint.Round(x + w * 0.07, y + w * 0.07, w * 0.86, h - w * 0.14, r * 0.8, 0xFF151515);
			m_Paint.Ellipse(x + w * 0.5, y + h * 0.5, w * 0.36, h * 0.3, 0xFFD7301F, 30);
			m_Paint.Ellipse(x + w * 0.5, y + h * 0.5, w * 0.2, h * 0.15, 0xFFE8B90C, 30);
			return;
		}

		int col = CardColor(code);
		m_Paint.Round(x + w * 0.07, y + w * 0.07, w * 0.86, h - w * 0.14, r * 0.8, col);
		m_Paint.Ellipse(x + w * 0.5, y + h * 0.5, w * 0.36, h * 0.32, 0xFFFFFFFF, 30);
		string label = Label(code);
		if (code.StartsWith("W"))
		{
			float qx = x + w * 0.5;
			float qy = y + h * 0.5;
			float q = w * 0.17;
			m_Paint.Circle(qx - q * 0.7, qy - q * 0.7, q * 0.7, 0xFFD7301F);
			m_Paint.Circle(qx + q * 0.7, qy - q * 0.7, q * 0.7, 0xFF1C5FC4);
			m_Paint.Circle(qx - q * 0.7, qy + q * 0.7, q * 0.7, 0xFFE8B90C);
			m_Paint.Circle(qx + q * 0.7, qy + q * 0.7, q * 0.7, 0xFF3C9A2E);
			if (code == "W4")
				TextMid(qx, qy, w, label, w * 0.26, 0xFFFFFFFF);
		}
		else
		{
			int size = w * 0.42;
			if (label.Length() > 2)
				size = w * 0.22;

			TextMid(x + w * 0.5, y + h * 0.5, w, label, size, col);
		}

		int corner = w * 0.18;
		if (label.Length() > 2)
			corner = w * 0.13;

		TextAt(x + w * 0.1, y + w * 0.08, w * 0.8, corner + 4, label, corner, 0xFFFFFFFF);
		if (dim)
			m_Paint.Round(x, y, w, h, r, 0x8C000000);
	}

	//------------------------------------------------------------------------------------------------
	override void Build(float x, float y, float w, float h)
	{
		Felt(x, y, w, h);
		array<string> hand = StrList("hand");
		array<int> ok = IntList("ok");
		array<int> counts = IntList("counts");

		// Opponents.
		int others = Seats() - 1;
		if (others < 1)
			others = 1;

		float slot = (w - 40) / others;
		int k;
		for (int s = 0; s < Seats(); s++)
		{
			if (s == m_iMe)
				continue;

			float sx = x + 20 + k * slot;
			k++;
			int n;
			if (counts.IsIndexValid(s))
				n = counts[s];

			if (s == m_iTurn && m_iWinner == -1)
				m_Paint.Round(sx + 4, y + 16, slot - 8, 108, 10, 0x40FFFFFF);

			int shown = n;
			if (shown > 8)
				shown = 8;

			float cw = 34;
			float step = 12;
			float fanW = cw + (shown - 1) * step;
			for (int i = 0; i < shown; i++)
			{
				Card(sx + (slot - fanW) * 0.5 + i * step, y + 24, cw, 50, "");
			}

			string name = SeatName(s);
			if (IsBot(s))
				name += " (bot)";

			TextMid(sx + slot * 0.5, y + 90, slot - 8, name, 14, 0xFFFFFFFF);
			TextMid(sx + slot * 0.5, y + 108, slot - 8, n.ToString() + " cards", 12, 0xFFCFE3D3);
		}

		// Piles.
		float cardW = 96;
		float cardH = 144;
		float midY = y + 140 + (h - 140 - 190 - cardH) * 0.5;
		float deckX = x + w * 0.5 - cardW - 30;
		float discX = x + w * 0.5 + 30;
		for (int i = 2; i >= 0; i--)
		{
			Card(deckX - i * 3, midY - i * 3, cardW, cardH, "");
		}

		TextMid(deckX + cardW * 0.5, midY + cardH + 14, cardW + 40, "Draw pile", 13, 0xFFCFE3D3);
		if (MyTurn() && m_iWild < 0)
			HitAt(deckX - 6, midY - 6, cardW + 6, cardH + 6, ID_CTRL + 10);

		string colour = S("colour");
		int ringCol = CardColor(colour + "0");
		m_Paint.Round(discX - 10, midY - 10, cardW + 20, cardH + 20, 16, ringCol);
		m_Paint.Round(discX - 5, midY - 5, cardW + 10, cardH + 10, 13, 0xFF1F6B3A);
		Card(discX, midY, cardW, cardH, S("top"));
		TextMid(discX + cardW * 0.5, midY + cardH + 14, cardW + 60, "Colour: " + ColourName(colour), 13, 0xFFFFFFFF);
		string dir = "Play goes forward";
		if (I("dir", 1) < 0)
			dir = "Play goes in reverse";

		TextMid(x + w * 0.5, midY + cardH + 38, w, dir, 12, 0xFFCFE3D3);

		// Own hand.
		int count = hand.Count();
		float hw = 84;
		float hh = 126;
		float hy = y + h - hh - 30;
		float hstep = hw + 8;
		if (count > 1 && hstep * (count - 1) + hw > w - 40)
			hstep = (w - 40 - hw) / (count - 1);

		float hx = x + (w - (hstep * (count - 1) + hw)) * 0.5;
		for (int i = 0; i < count; i++)
		{
			bool can = ok.IsIndexValid(i) && ok[i] == 1;
			float cy = hy;
			if (can)
				cy -= 18;

			if (i == m_iWild)
				cy -= 30;

			Card(hx + i * hstep, cy, hw, hh, hand[i], MyTurn() && !can);
			if (can && m_iWild < 0)
			{
				float hitW = hstep;
				if (i == count - 1)
					hitW = hw;

				HitAt(hx + i * hstep, cy, hitW, hh, ID_BASE + i);
			}
		}

		if (m_iWild >= 0)
		{
			m_Paint.Round(x + w * 0.5 - 170, midY + 10, 340, 120, 14, 0xE0101010);
			TextMid(x + w * 0.5, midY + 34, 320, "Choose the colour", 16, 0xFFFFFFFF);
			array<string> cs = {"r", "g", "b", "y"};
			foreach (int i, string c : cs)
			{
				float bx = x + w * 0.5 - 150 + i * 78;
				m_Paint.Circle(bx + 30, midY + 88, 28, 0xFFFFFFFF);
				m_Paint.Circle(bx + 30, midY + 88, 25, CardColor(c + "0"));
				HitAt(bx, midY + 58, 60, 60, ID_CTRL + i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override float BuildSide(float x, float y, float w)
	{
		float cy = y;
		TextAt(x, cy, w, 20, string.Format("Cards in the draw pile: %1", I("deck")), 14, 0xFFE0E0E0);
		cy += 28;
		if (MyTurn())
		{
			ButtonAt(x, cy, w, 34, "Draw a card", ID_CTRL + 10);
			cy += 42;
			if (m_iWild >= 0)
			{
				ButtonAt(x, cy, w, 30, "Cancel wild", ID_CTRL + 5);
				cy += 38;
			}
		}

		m_sHint = "";
		if (MyTurn())
			m_sHint = "Click a raised card to play it, or the draw pile to draw.";

		return cy - y;
	}

	//------------------------------------------------------------------------------------------------
	override string Click(int id, out bool redraw)
	{
		redraw = true;
		if (id == ID_CTRL + 10)
			return "draw";

		if (id == ID_CTRL + 5)
		{
			m_iWild = -1;
			return "";
		}

		array<string> hand = StrList("hand");
		if (m_iWild >= 0 && id >= ID_CTRL && id < ID_CTRL + 4)
		{
			array<string> cs = {"r", "g", "b", "y"};
			string wild = "";
			if (hand.IsIndexValid(m_iWild))
				wild = hand[m_iWild];

			m_iWild = -1;
			if (wild.IsEmpty())
				return "";

			return "play " + wild + " " + cs[id - ID_CTRL];
		}

		int i = id - ID_BASE;
		if (!hand.IsIndexValid(i))
		{
			redraw = false;
			return "";
		}

		if (hand[i].StartsWith("W"))
		{
			m_iWild = i;
			return "";
		}

		return "play " + hand[i];
	}

	//------------------------------------------------------------------------------------------------
	override void Preview(float x, float y, float w, float h)
	{
		float ch = h * 0.86;
		float cw = ch * 0.66;
		float cx = x + w * 0.5 - cw * 1.2;
		Card(cx, y + h * 0.1, cw, ch, "r7");
		Card(cx + cw * 0.7, y + h * 0.04, cw, ch, "bR");
		Card(cx + cw * 1.4, y + h * 0.1, cw, ch, "W4");
	}
}

//! Blackjack table: dealer on top, seats below; Deal, Hit and Stand in the side panel.
class ROOT_AE3_GameBoardBlackjack : ROOT_AE3_GameBoard
{
	//------------------------------------------------------------------------------------------------
	protected void Hand(float x, float y, float cw, float ch, notnull array<string> cards)
	{
		foreach (int i, string c : cards)
		{
			PlayingCard(x + i * cw * 0.42, y, cw, ch, c);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Build(float x, float y, float w, float h)
	{
		Felt(x, y, w, h);
		array<string> dealer = StrList("dealer");
		float cw = 78;
		float ch = 112;
		float dw = cw + (dealer.Count() - 1) * cw * 0.42;
		TextMid(x + w * 0.5, y + 30, w, "DEALER", 15, 0xFFE8D7A0);
		Hand(x + (w - dw) * 0.5, y + 48, cw, ch, dealer);
		int dv = I("dv", -1);
		if (dv >= 0)
			TextMid(x + w * 0.5, y + 48 + ch + 16, w, dv.ToString(), 18, 0xFFFFFFFF);

		m_Paint.Ellipse(x + w * 0.5, y + h * 0.47, w * 0.32, 16, 0x20FFFFFF);
		TextMid(x + w * 0.5, y + h * 0.47, w, "BLACKJACK PAYS ONE POINT - FIRST TO FIVE", 12, 0x99FFFFFF);

		array<int> values = IntList("v");
		array<int> score = IntList("score");
		array<int> done = IntList("done");
		bool round = I("round") == 1;
		int n = Seats();
		if (n < 1)
			return;

		float slot = (w - 30) / n;
		float scw = cw;
		float sch = ch;
		if (slot < 200)
		{
			scw = cw * 0.72;
			sch = ch * 0.72;
		}

		float sy = y + h - sch - 96;
		for (int s = 0; s < n; s++)
		{
			float sx = x + 15 + s * slot;
			if (s == m_iTurn && round && m_iWinner == -1)
				m_Paint.Round(sx + 4, sy - 34, slot - 8, sch + 120, 12, 0x40FFFFFF);

			if (s == m_iMe)
				m_Paint.Round(sx + 4, sy - 34, slot - 8, 4, 2, 0xFFE8C330);

			array<string> cards = StrList("h" + s.ToString());
			float hw = scw + (cards.Count() - 1) * scw * 0.42;
			Hand(sx + (slot - hw) * 0.5, sy, scw, sch, cards);
			string name = SeatName(s);
			if (IsBot(s))
				name += " (bot)";

			TextMid(sx + slot * 0.5, sy - 18, slot - 8, name, 14, 0xFFFFFFFF);
			string info = "";
			if (values.IsIndexValid(s) && values[s] > 0)
			{
				info = values[s].ToString();
				if (values[s] > 21)
					info += " bust";
				else if (round && done.IsIndexValid(s) && done[s] == 1)
					info += " stands";
			}

			TextMid(sx + slot * 0.5, sy + sch + 16, slot - 8, info, 16, 0xFFFFFFFF);
			int pts;
			if (score.IsIndexValid(s))
				pts = score[s];

			for (int p = 0; p < 5; p++)
			{
				int col = 0x40FFFFFF;
				if (p < pts)
					col = 0xFFE8C330;

				m_Paint.Disc(sx + slot * 0.5 - 40 + p * 20, sy + sch + 46, 7, col, 0xFF6B5A1C, 1);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override float BuildSide(float x, float y, float w)
	{
		float cy = y;
		bool round = I("round") == 1;
		if (m_iWinner == -1 && !round)
		{
			ButtonAt(x, cy, w, 40, "Deal the next round", ID_CTRL + 2, true);
			cy += 48;
		}

		if (MyTurn())
		{
			float bw = (w - 8) * 0.5;
			ButtonAt(x, cy, bw, 40, "Hit", ID_CTRL, true);
			ButtonAt(x + bw + 8, cy, bw, 40, "Stand", ID_CTRL + 1);
			cy += 48;
		}

		string result = S("result");
		if (!result.IsEmpty())
		{
			TextAt(x, cy, w, 64, result, 13, 0xFFE0E0E0, 0, true);
			cy += 70;
		}

		m_sHint = "";
		if (MyTurn())
			m_sHint = "Hit takes a card, Stand keeps your total. Do not pass 21.";
		else if (!round && m_iWinner == -1)
			m_sHint = "Any player may deal the next round.";

		return cy - y;
	}

	//------------------------------------------------------------------------------------------------
	override string Click(int id, out bool redraw)
	{
		redraw = false;
		switch (id)
		{
			case ID_CTRL: return "hit";
			case ID_CTRL + 1: return "stand";
			case ID_CTRL + 2: return "deal";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void Preview(float x, float y, float w, float h)
	{
		float ch = h * 0.86;
		float cw = ch * 0.7;
		PlayingCard(x + w * 0.5 - cw * 0.9, y + h * 0.1, cw, ch, "As");
		PlayingCard(x + w * 0.5 - cw * 0.2, y + h * 0.04, cw, ch, "Kh");
	}
}

//! Rock paper scissors: three big throws; the opponent's throw shows when the round is decided.
class ROOT_AE3_GameBoardRps : ROOT_AE3_GameBoard
{
	//------------------------------------------------------------------------------------------------
	//! rock, paper or scissors in the box x, y, s; empty draws a question mark card.
	protected void Throw(string t, float x, float y, float s)
	{
		switch (t)
		{
			case "rock":
			{
				array<float> v = {0.2, 0.55, 0.3, 0.3, 0.55, 0.22, 0.78, 0.35, 0.84, 0.6, 0.66, 0.8, 0.36, 0.8};
				m_Paint.Shape(x + s * 0.03, y + s * 0.05, s, v, 0x55000000, 0x00000000, 0.1);
				m_Paint.Shape(x, y, s, v, 0xFF8E8A84, 0xFF3C3A36, 2);
				array<float> hi = {0.32, 0.36, 0.52, 0.29, 0.6, 0.4, 0.38, 0.47};
				m_Paint.Shape(x, y, s, hi, 0xFFB3AFA8, 0xFFB3AFA8, 0.5);
				return;
			}

			case "paper":
			{
				array<float> v = {0.24, 0.16, 0.7, 0.16, 0.8, 0.27, 0.8, 0.86, 0.24, 0.86};
				m_Paint.Shape(x + s * 0.03, y + s * 0.04, s, v, 0x55000000, 0x00000000, 0.1);
				m_Paint.Shape(x, y, s, v, 0xFFF7F7F2, 0xFF5A5A5A, 2);
				for (int i = 0; i < 5; i++)
				{
					float ly = y + s * (0.36 + i * 0.1);
					m_Paint.Segment(x + s * 0.32, ly, x + s * 0.72, ly, 1.5, 0xFF8AA8C8);
				}

				return;
			}

			case "scissors":
			{
				array<float> b1 = {0.46, 0.5, 0.52, 0.46, 0.86, 0.12, 0.84, 0.1};
				array<float> b2 = {0.46, 0.46, 0.52, 0.5, 0.16, 0.12, 0.14, 0.14};
				m_Paint.Shape(x, y, s, b1, 0xFFC9D1D9, 0xFF4A525A, 1.5);
				m_Paint.Shape(x, y, s, b2, 0xFFC9D1D9, 0xFF4A525A, 1.5);
				m_Paint.Ring(x + s * 0.3, y + s * 0.72, s * 0.13, s * 0.06, 0xFFD23B3B);
				m_Paint.Ring(x + s * 0.7, y + s * 0.72, s * 0.13, s * 0.06, 0xFFD23B3B);
				m_Paint.Segment(x + s * 0.49, y + s * 0.48, x + s * 0.36, y + s * 0.62, s * 0.06, 0xFFD23B3B);
				m_Paint.Segment(x + s * 0.49, y + s * 0.48, x + s * 0.64, y + s * 0.62, s * 0.06, 0xFFD23B3B);
				m_Paint.Circle(x + s * 0.49, y + s * 0.48, s * 0.03, 0xFF303030);
				return;
			}
		}

		m_Paint.Round(x + s * 0.18, y + s * 0.1, s * 0.64, s * 0.8, s * 0.08, 0xFF2D3E50);
		TextMid(x + s * 0.5, y + s * 0.5, s, "?", s * 0.4, 0xFFFFFFFF);
	}

	//------------------------------------------------------------------------------------------------
	override void Build(float x, float y, float w, float h)
	{
		m_Paint.Round(x, y, w, h, 16, 0xFF233042);
		array<int> score = IntList("score");
		array<string> last = StrList("last");
		string mine = S("mine");
		bool theirs = I("theirs") == 1;
		int a;
		int b;
		if (score.Count() == 2)
		{
			a = score[0];
			b = score[1];
		}

		string opponent = SeatName(1 - m_iMe);
		TextMid(x + w * 0.5, y + 28, w, "Round " + S("round") + "  -  first to 3", 14, 0xFFB8C4D4);
		TextMid(x + w * 0.25, y + 60, w * 0.5, "You", 18, 0xFFFFFFFF);
		TextMid(x + w * 0.75, y + 60, w * 0.5, opponent, 18, 0xFFFFFFFF);
		TextMid(x + w * 0.5, y + 110, 200, a.ToString() + " : " + b.ToString(), 44, 0xFFFFFFFF);
		float s = h * 0.32;
		if (s > w * 0.3)
			s = w * 0.3;

		float py = y + 90;
		string myShown = mine;
		string theirShown = "";
		string caption = "";
		if ((mine.IsEmpty() || m_iWinner != -1) && last.Count() == 2)
		{
			myShown = last[0];
			theirShown = last[1];
			caption = "Last round";
		}

		m_Paint.Circle(x + w * 0.25, py + s * 0.5, s * 0.55, 0x22FFFFFF);
		m_Paint.Circle(x + w * 0.75, py + s * 0.5, s * 0.55, 0x22FFFFFF);
		if (!myShown.IsEmpty())
			Throw(myShown, x + w * 0.25 - s * 0.5, py, s);

		if (!theirShown.IsEmpty())
			Throw(theirShown, x + w * 0.75 - s * 0.5, py, s);
		else if (theirs)
			Throw("", x + w * 0.75 - s * 0.5, py, s);

		string waiting = "Waiting...";
		if (theirs)
			waiting = "Thrown";

		if (m_iWinner == -1)
			TextMid(x + w * 0.75, py + s + 26, w * 0.5, "This round: " + waiting, 14, 0xFFB8C4D4);

		if (!caption.IsEmpty())
			TextMid(x + w * 0.5, py + s + 52, w, caption, 13, 0xFFB8C4D4);

		// Choices.
		float bs = (w - 80) / 3;
		if (bs > h * 0.28)
			bs = h * 0.28;

		float by = y + h - bs - 40;
		array<string> names = {"rock", "paper", "scissors"};
		bool can = mine.IsEmpty() && m_iWinner == -1;
		foreach (int i, string n : names)
		{
			float bx = x + (w - bs * 3 - 40) * 0.5 + i * (bs + 20);
			int bg = 0xFF2F4058;
			if (n == mine)
				bg = 0xFF3D7BE0;

			m_Paint.Round(bx, by, bs, bs, 14, bg);
			Throw(n, bx + bs * 0.12, by + bs * 0.06, bs * 0.76);
			string label = n;
			label.ToUpper();
			TextMid(bx + bs * 0.5, by + bs + 14, bs, label, 13, 0xFFE0E6EE);
			if (can)
				HitAt(bx, by, bs, bs, ID_CTRL + i);
		}
	}

	//------------------------------------------------------------------------------------------------
	override float BuildSide(float x, float y, float w)
	{
		m_sHint = "";
		if (S("mine").IsEmpty() && m_iWinner == -1)
			m_sHint = "Click rock, paper or scissors. Both throws show when the round is decided.";
		else if (m_iWinner == -1)
			m_sHint = "You threw " + S("mine") + ". Waiting for the other throw.";

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override string Click(int id, out bool redraw)
	{
		redraw = false;
		array<string> names = {"rock", "paper", "scissors"};
		int i = id - ID_CTRL;
		if (i >= 0 && i < 3)
			return names[i];

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void Preview(float x, float y, float w, float h)
	{
		float s = h * 0.9;
		Throw("rock", x + w * 0.5 - s * 1.5, y, s);
		Throw("paper", x + w * 0.5 - s * 0.5, y, s);
		Throw("scissors", x + w * 0.5 + s * 0.5, y, s);
	}
}
