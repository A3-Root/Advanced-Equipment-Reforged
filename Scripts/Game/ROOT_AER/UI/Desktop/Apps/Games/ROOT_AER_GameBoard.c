//! Graphical board of one game in the Games app. Reads the table state sent by the server (game_view field 12:
//! key=value pairs joined by ';'), draws the board on a canvas and turns clicks into the moves the server checks.
class ROOT_AER_GameBoard
{
	static const int ID_BASE = 1000;	//!< board cells: ID_BASE + cell
	static const int ID_CTRL = 1300;	//!< controls: ID_CTRL + n

	protected ROOT_AER_AppGames m_App;
	protected ref map<string, string> m_mState = new map<string, string>();
	protected string m_sState;
	protected ref array<string> m_aNames = {};
	protected ref ROOT_AER_GamePaint m_Paint;
	protected Widget m_wParent;
	protected string m_sHint;

	string m_sGame;
	int m_iMe;
	int m_iTurn = -1;
	int m_iWinner = -1;

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_GameBoard Create(string game, ROOT_AER_AppGames app)
	{
		ROOT_AER_GameBoard b;
		switch (game)
		{
			case "chess": b = new ROOT_AER_GameBoardChess(); break;
			case "checkers": b = new ROOT_AER_GameBoardCheckers(); break;
			case "uno": b = new ROOT_AER_GameBoardUno(); break;
			case "ludo": b = new ROOT_AER_GameBoardLudo(); break;
			case "snakesladders": b = new ROOT_AER_GameBoardSnakes(); break;
			case "blackjack": b = new ROOT_AER_GameBoardBlackjack(); break;
			case "rps": b = new ROOT_AER_GameBoardRps(); break;
			case "battleship": b = new ROOT_AER_GameBoardBattleship(); break;
			default: b = new ROOT_AER_GameBoard(); break;
		}

		b.m_App = app;
		b.m_sGame = game;
		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! New state from the server; local selections are kept while the state is unchanged.
	void SetState(string gui, notnull array<string> names)
	{
		m_aNames.Clear();
		m_aNames.Copy(names);
		if (gui == m_sState)
			return;

		m_sState = gui;
		m_mState.Clear();
		array<string> pairs = {};
		gui.Split(";", pairs, true);
		foreach (string p : pairs)
		{
			int eq = p.IndexOf("=");
			if (eq <= 0)
				continue;

			m_mState.Set(p.Substring(0, eq), p.Substring(eq + 1, p.Length() - eq - 1));
		}

		m_iMe = I("me");
		m_iTurn = I("turn", -1);
		m_iWinner = I("winner", -1);
		OnNewState();
	}

	//------------------------------------------------------------------------------------------------
	bool HasState()
	{
		return !m_mState.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNewState()
	{
	}

	//------------------------------------------------------------------------------------------------
	string S(string key)
	{
		return m_mState.Get(key);
	}

	//------------------------------------------------------------------------------------------------
	int I(string key, int def = 0)
	{
		string v = m_mState.Get(key);
		if (v.IsEmpty())
			return def;

		return v.ToInt();
	}

	//------------------------------------------------------------------------------------------------
	array<int> IntList(string key)
	{
		array<int> r = {};
		array<string> parts = StrList(key);
		foreach (string p : parts)
		{
			r.Insert(p.ToInt());
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	array<string> StrList(string key)
	{
		array<string> parts = {};
		string v = m_mState.Get(key);
		if (!v.IsEmpty())
			v.Split(",", parts, true);

		return parts;
	}

	//------------------------------------------------------------------------------------------------
	bool MyTurn()
	{
		return m_iWinner == -1 && m_iTurn == m_iMe;
	}

	//------------------------------------------------------------------------------------------------
	int Seats()
	{
		return m_aNames.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool IsBot(int seat)
	{
		array<int> b = IntList("bots");
		return b.IsIndexValid(seat) && b[seat] == 1;
	}

	//------------------------------------------------------------------------------------------------
	bool IsMine(int seat)
	{
		array<int> m = IntList("mine");
		return m.IsIndexValid(seat) && m[seat] == 1;
	}

	//------------------------------------------------------------------------------------------------
	string SeatName(int seat)
	{
		if (m_aNames.IsIndexValid(seat))
			return m_aNames[seat];

		return "Seat " + (seat + 1).ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! Color of a seat: red, blue, green, yellow, purple, orange, teal, pink.
	int SeatColor(int seat)
	{
		array<int> c = {0xFFE04848, 0xFF3D7BE0, 0xFF3BAA4F, 0xFFE8C330, 0xFF9B5DE0, 0xFFF08A2C, 0xFF2BB5AA, 0xFFE061A8};
		if (seat < 0)
			return 0xFF808080;

		return c[seat % 8];
	}

	//------------------------------------------------------------------------------------------------
	//! Text for the side panel under the controls (what to click).
	string Hint()
	{
		return m_sHint;
	}

	//------------------------------------------------------------------------------------------------
	//! Starts drawing: one canvas over the whole parent, under the widgets made afterwards.
	void Begin(Widget parent, float w, float h)
	{
		m_wParent = parent;
		m_Paint = new ROOT_AER_GamePaint(parent, 0, 0, w, h);
	}

	//------------------------------------------------------------------------------------------------
	void End()
	{
		if (m_Paint)
			m_Paint.Commit();
	}

	//------------------------------------------------------------------------------------------------
	//! Board area.
	void Build(float x, float y, float w, float h)
	{
		TextAt(x + 20, y + 20, w - 40, h - 40, "This game has no picture board; use the move field.", 15, 0xFFB0B0B0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Controls in the side panel; returns the height used.
	float BuildSide(float x, float y, float w)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Decorative picture of the game for the lobby.
	void Preview(float x, float y, float w, float h)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Click on a cell or control: the move to send, or empty. redraw: the board changed locally.
	string Click(int id, out bool redraw)
	{
		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected void HitAt(float x, float y, float w, float h, int id)
	{
		ROOT_AER_UI.Hit(m_wParent, x, y, w, h, id, m_App, ROOT_AER_UI.Rgb(0, 0, 0, 0), ROOT_AER_UI.Rgb(1, 1, 1, 0.18));
	}

	//------------------------------------------------------------------------------------------------
	//! align: 0 left, 1 center, 2 right.
	protected TextWidget TextAt(float x, float y, float w, float h, string text, int size, int color, int align = 0, bool wrap = false)
	{
		return ROOT_AER_UI.Text(m_wParent, x, y, w, h, text, size, Color.FromInt(color), false, wrap, align);
	}

	//------------------------------------------------------------------------------------------------
	//! Text centered on cx, cy.
	protected TextWidget TextMid(float cx, float cy, float w, string text, int size, int color)
	{
		return TextAt(cx - w * 0.5, cy - size * 0.62, w, size * 1.4, text, size, color, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void ButtonAt(float x, float y, float w, float h, string label, int id, bool main = false)
	{
		if (main)
		{
			ROOT_AER_UI.Button(m_wParent, x, y, w, h, label, id, m_App, ROOT_AER_Theme.Accent(), ROOT_AER_UI.Rgb(1, 1, 1), 15);
			return;
		}

		ROOT_AER_UI.Rect(m_wParent, x, y, w, h, ROOT_AER_Theme.Border());
		ROOT_AER_UI.Button(m_wParent, x + 1, y + 1, w - 2, h - 2, label, id, m_App, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Text(), 15);
	}

	//------------------------------------------------------------------------------------------------
	//! Table cloth with a darker rim.
	protected void Felt(float x, float y, float w, float h)
	{
		m_Paint.Round(x, y, w, h, 18, 0xFF3B2414);
		m_Paint.Round(x + 8, y + 8, w - 16, h - 16, 14, 0xFF1F6B3A);
		m_Paint.Ellipse(x + w * 0.5, y + h * 0.45, w * 0.38, h * 0.3, 0x14FFFFFF);
	}

	//------------------------------------------------------------------------------------------------
	//! Die face with pips; value 0 draws a blank die.
	protected void Die(float x, float y, float s, int value)
	{
		m_Paint.Round(x + 3, y + 4, s, s, s * 0.18, 0x55000000);
		m_Paint.Round(x, y, s, s, s * 0.18, 0xFF202020);
		m_Paint.Round(x + 2, y + 2, s - 4, s - 4, s * 0.16, 0xFFF7F7F2);
		array<float> px = {};
		array<float> py = {};
		float a = 0.27;
		float b = 0.5;
		float c = 0.73;
		switch (value)
		{
			case 1: px = {b}; py = {b}; break;
			case 2: px = {a, c}; py = {a, c}; break;
			case 3: px = {a, b, c}; py = {a, b, c}; break;
			case 4: px = {a, c, a, c}; py = {a, a, c, c}; break;
			case 5: px = {a, c, b, a, c}; py = {a, a, b, c, c}; break;
			case 6: px = {a, c, a, c, a, c}; py = {a, a, b, b, c, c}; break;
		}

		int pip = 0xFF1A1A1A;
		if (value == 1)
			pip = 0xFFC81E1E;

		foreach (int i, float ux : px)
		{
			m_Paint.Circle(x + ux * s, y + py[i] * s, s * 0.085, pip);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Card suit symbol: c clubs, d diamonds, h hearts, s spades.
	protected void Suit(string suit, float cx, float cy, float s, int color)
	{
		switch (suit)
		{
			case "h":
			{
				m_Paint.Circle(cx - s * 0.24, cy - s * 0.14, s * 0.27, color);
				m_Paint.Circle(cx + s * 0.24, cy - s * 0.14, s * 0.27, color);
				m_Paint.Tri(cx - s * 0.5, cy - s * 0.04, cx + s * 0.5, cy - s * 0.04, cx, cy + s * 0.55, color);
				return;
			}

			case "d":
			{
				array<float> v = {cx, cy - s * 0.55, cx + s * 0.4, cy, cx, cy + s * 0.55, cx - s * 0.4, cy};
				m_Paint.Poly(v, color);
				return;
			}

			case "s":
			{
				m_Paint.Circle(cx - s * 0.24, cy + s * 0.1, s * 0.25, color);
				m_Paint.Circle(cx + s * 0.24, cy + s * 0.1, s * 0.25, color);
				m_Paint.Tri(cx - s * 0.49, cy + s * 0.04, cx + s * 0.49, cy + s * 0.04, cx, cy - s * 0.55, color);
				m_Paint.Tri(cx, cy + s * 0.1, cx + s * 0.17, cy + s * 0.58, cx - s * 0.17, cy + s * 0.58, color);
				return;
			}

			case "c":
			{
				m_Paint.Circle(cx, cy - s * 0.27, s * 0.23, color);
				m_Paint.Circle(cx - s * 0.26, cy + s * 0.08, s * 0.23, color);
				m_Paint.Circle(cx + s * 0.26, cy + s * 0.08, s * 0.23, color);
				m_Paint.Tri(cx, cy - s * 0.05, cx + s * 0.17, cy + s * 0.58, cx - s * 0.17, cy + s * 0.58, color);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Playing card "10h", "As"; "?" draws the back.
	protected void PlayingCard(float x, float y, float w, float h, string code)
	{
		m_Paint.Round(x + 3, y + 4, w, h, w * 0.1, 0x55000000);
		if (code == "?" || code.Length() < 2)
		{
			m_Paint.Round(x, y, w, h, w * 0.1, 0xFFF5F5F5);
			m_Paint.Round(x + 4, y + 4, w - 8, h - 8, w * 0.08, 0xFF8E1B1B);
			for (int i = 1; i < 6; i++)
			{
				m_Paint.Segment(x + 6, y + h * i / 6, x + w - 6, y + h * i / 6, 1, 0x40FFFFFF);
			}

			m_Paint.Ellipse(x + w * 0.5, y + h * 0.5, w * 0.24, h * 0.18, 0x55FFFFFF);
			return;
		}

		string suit = code.Substring(code.Length() - 1, 1);
		string rank = code.Substring(0, code.Length() - 1);
		int col = 0xFF1A1A1A;
		if (suit == "h" || suit == "d")
			col = 0xFFC81E1E;

		m_Paint.Round(x, y, w, h, w * 0.1, 0xFFBDBDBD);
		m_Paint.Round(x + 1, y + 1, w - 2, h - 2, w * 0.09, 0xFFFFFFFF);
		int size = w * 0.26;
		TextAt(x + 4, y + 3, w * 0.5, size + 4, rank, size, col);
		Suit(suit, x + 4 + size * 0.45, y + size * 1.75, size * 0.7, col);
		Suit(suit, x + w * 0.5, y + h * 0.58, w * 0.42, col);
	}
}
