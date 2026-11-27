//! Ludo board (15 x 15 cells): yards in the corners, shared track, home columns, home triangle in the middle.
//! Roll in the side panel, then click a highlighted token.
class ROOT_AER_GameBoardLudo : ROOT_AER_GameBoard
{
	protected static ref array<int> s_aTrack;

	//------------------------------------------------------------------------------------------------
	//! Track cells (x, y pairs) from seat 1's start square, clockwise.
	protected static array<int> Track()
	{
		if (!s_aTrack)
		{
			s_aTrack = {1,6, 2,6, 3,6, 4,6, 5,6, 6,5, 6,4, 6,3, 6,2, 6,1, 6,0, 7,0, 8,0, 8,1, 8,2, 8,3, 8,4, 8,5,
				9,6, 10,6, 11,6, 12,6, 13,6, 14,6, 14,7, 14,8, 13,8, 12,8, 11,8, 10,8, 9,8, 8,9, 8,10, 8,11, 8,12, 8,13, 8,14,
				7,14, 6,14, 6,13, 6,12, 6,11, 6,10, 6,9, 5,8, 4,8, 3,8, 2,8, 1,8, 0,8, 0,7, 0,6};
		}

		return s_aTrack;
	}

	//------------------------------------------------------------------------------------------------
	//! Red, green, yellow, blue.
	override int SeatColor(int seat)
	{
		array<int> c = {0xFFE04848, 0xFF3BAA4F, 0xFFE8C330, 0xFF3D7BE0};
		if (seat < 0)
			return 0xFF808080;

		return c[seat % 4];
	}

	//------------------------------------------------------------------------------------------------
	//! Point in board units (0-15) turned by quarter turns clockwise.
	protected static void Turn(float ux, float uy, int quarters, out float rx, out float ry)
	{
		rx = ux;
		ry = uy;
		for (int i = 0; i < quarters % 4; i++)
		{
			float t = rx;
			rx = 15 - ry;
			ry = t;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CellAt(float ox, float oy, float cell, float ux, float uy, int quarters, int color)
	{
		float rx;
		float ry;
		Turn(ux + 0.5, uy + 0.5, quarters, rx, ry);
		m_Paint.Rect(ox + (rx - 0.5) * cell + 1, oy + (ry - 0.5) * cell + 1, cell - 2, cell - 2, color);
	}

	//------------------------------------------------------------------------------------------------
	//! Centre of a token in board units.
	protected void TokenPos(int seat, int token, int progress, out float ux, out float uy)
	{
		if (progress < 0)
		{
			array<float> yx = {2, 4, 2, 4};
			array<float> yy = {2, 2, 4, 4};
			Turn(yx[token], yy[token], seat, ux, uy);
			return;
		}

		if (progress <= 51)
		{
			int idx = (progress + seat * 13) % 52;
			ux = Track()[idx * 2] + 0.5;
			uy = Track()[idx * 2 + 1] + 0.5;
			return;
		}

		if (progress <= 56)
		{
			Turn(progress - 51 + 0.5, 7.5, seat, ux, uy);
			return;
		}

		array<float> hx = {6.55, 6.55, 6.95, 6.95};
		array<float> hy = {7.2, 7.8, 7.2, 7.8};
		Turn(hx[token], hy[token], seat, ux, uy);
	}

	//------------------------------------------------------------------------------------------------
	override void Build(float x, float y, float w, float h)
	{
		float size = w;
		if (h < size)
			size = h;

		size -= 20;
		float cell = size / 15;
		float ox = x + (w - size) * 0.5;
		float oy = y + (h - size) * 0.5;
		m_Paint.Round(ox - 8, oy - 6, size + 16, size + 16, 12, 0x66000000);
		m_Paint.Round(ox - 10, oy - 10, size + 20, size + 20, 12, 0xFF3A3A3A);
		m_Paint.Rect(ox, oy, size, size, 0xFFFAFAF5);

		for (int q = 0; q < 4; q++)
		{
			int col = SeatColor(q);
			float ax;
			float ay;
			float bx;
			float by;
			Turn(0, 0, q, ax, ay);
			Turn(6, 6, q, bx, by);
			float lx = Math.Min(ax, bx);
			float ly = Math.Min(ay, by);
			m_Paint.Rect(ox + lx * cell, oy + ly * cell, cell * 6, cell * 6, col);
			m_Paint.Round(ox + (lx + 1) * cell, oy + (ly + 1) * cell, cell * 4, cell * 4, cell * 0.4, 0xFFFAFAF5);
			array<float> sx = {2, 4, 2, 4};
			array<float> sy = {2, 2, 4, 4};
			for (int t = 0; t < 4; t++)
			{
				float px;
				float py;
				Turn(sx[t], sy[t], q, px, py);
				m_Paint.Circle(ox + px * cell, oy + py * cell, cell * 0.55, col);
				m_Paint.Circle(ox + px * cell, oy + py * cell, cell * 0.42, 0xFFFAFAF5);
			}

			// Start square and home column.
			CellAt(ox, oy, cell, 1, 6, q, col);
			for (int c = 1; c <= 5; c++)
			{
				CellAt(ox, oy, cell, c, 7, q, col);
			}

			float t1x;
			float t1y;
			float t2x;
			float t2y;
			Turn(6, 6, q, t1x, t1y);
			Turn(6, 9, q, t2x, t2y);
			m_Paint.Tri(ox + t1x * cell, oy + t1y * cell, ox + t2x * cell, oy + t2y * cell, ox + 7.5 * cell, oy + 7.5 * cell, col);
		}

		// Grid lines of the track arms.
		for (int i = 0; i <= 15; i++)
		{
			float gx = ox + i * cell;
			float gy = oy + i * cell;
			if (i >= 6 && i <= 9)
			{
				m_Paint.Segment(gx, oy, gx, oy + 6 * cell, 1, 0xFF9A9A9A);
				m_Paint.Segment(gx, oy + 9 * cell, gx, oy + size, 1, 0xFF9A9A9A);
				m_Paint.Segment(ox, gy, ox + 6 * cell, gy, 1, 0xFF9A9A9A);
				m_Paint.Segment(ox + 9 * cell, gy, ox + size, gy, 1, 0xFF9A9A9A);
			}
			else
			{
				m_Paint.Segment(ox + 6 * cell, gy, ox + 9 * cell, gy, 1, 0xFF9A9A9A);
				m_Paint.Segment(gx, oy + 6 * cell, gx, oy + 9 * cell, 1, 0xFF9A9A9A);
			}
		}

		// Tokens; tokens sharing a cell are spread out.
		array<int> tokens = IntList("tokens");
		array<int> can = IntList("can");
		map<string, int> stack = new map<string, int>();
		for (int s = 0; s < tokens.Count() / 4; s++)
		{
			for (int t = 0; t < 4; t++)
			{
				int p = tokens[s * 4 + t];
				float ux;
				float uy;
				TokenPos(s, t, p, ux, uy);
				string key = string.Format("%1_%2", Math.Round(ux * 10), Math.Round(uy * 10));
				int n = stack.Get(key);
				stack.Set(key, n + 1);
				int odd = n % 2;
				int pair = n / 2;
				float cx = ox + ux * cell + odd * cell * 0.22 - pair * cell * 0.1;
				float cy = oy + uy * cell - odd * cell * 0.16;
				bool movable = s == m_iMe && can.IsIndexValid(t) && can[t] == 1;
				Token(cx, cy, cell * 0.36, SeatColor(s), movable);
				if (movable)
					HitAt(cx - cell * 0.5, cy - cell * 0.5, cell, cell, ID_BASE + t);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Token(float cx, float cy, float r, int color, bool glow)
	{
		if (glow)
		{
			m_Paint.Circle(cx, cy, r * 1.6, 0x66FFE14D);
			m_Paint.Ring(cx, cy, r * 1.35, r * 0.18, 0xFFFFC400);
		}

		m_Paint.Circle(cx + r * 0.12, cy + r * 0.2, r, 0x66000000);
		m_Paint.Disc(cx, cy, r, color, 0xFF202020, r * 0.12);
		m_Paint.Circle(cx, cy, r * 0.55, 0xFFFFFFFF);
		m_Paint.Circle(cx, cy, r * 0.38, color);
	}

	//------------------------------------------------------------------------------------------------
	override float BuildSide(float x, float y, float w)
	{
		int roll = I("roll");
		int shown = I("shown");
		Die(x + (w - 90) * 0.5, y, 90, shown);
		float cy = y + 104;
		if (MyTurn() && roll == 0)
		{
			ButtonAt(x, cy, w, 40, "Roll the die", ID_CTRL, true);
			cy += 48;
		}

		m_sHint = "";
		if (MyTurn() && roll == 0)
			m_sHint = "Roll the die. A 6 brings a token out of the yard and rolls again.";
		else if (MyTurn())
			m_sHint = "Click a glowing token to move it " + roll.ToString() + ".";

		return cy - y;
	}

	//------------------------------------------------------------------------------------------------
	override string Click(int id, out bool redraw)
	{
		redraw = false;
		if (id == ID_CTRL)
			return "roll";

		int t = id - ID_BASE;
		if (t >= 0 && t < 4)
			return "move " + (t + 1).ToString();

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void Preview(float x, float y, float w, float h)
	{
		float s = h * 0.7;
		Die(x + w * 0.5 - s * 1.2, y + h * 0.15, s, 6);
		Token(x + w * 0.5 + s * 0.4, y + h * 0.4, s * 0.28, SeatColor(0), false);
		Token(x + w * 0.5 + s * 0.9, y + h * 0.6, s * 0.28, SeatColor(1), false);
	}
}

//! Snakes and ladders board (10 x 10, square 1 bottom left).
class ROOT_AER_GameBoardSnakes : ROOT_AER_GameBoard
{
	//------------------------------------------------------------------------------------------------
	protected static void Center(int n, float ox, float oy, float cell, out float cx, out float cy)
	{
		int r = (n - 1) / 10;
		int c = (n - 1) % 10;
		if (r % 2 == 1)
			c = 9 - c;

		cx = ox + c * cell + cell * 0.5;
		cy = oy + (9 - r) * cell + cell * 0.5;
	}

	//------------------------------------------------------------------------------------------------
	protected void Ladder(float ax, float ay, float bx, float by, float cell)
	{
		float dx = bx - ax;
		float dy = by - ay;
		float len = Math.Sqrt(dx * dx + dy * dy);
		if (len < 1)
			return;

		float nx = -dy / len * cell * 0.16;
		float ny = dx / len * cell * 0.16;
		int rungs = len / (cell * 0.32);
		for (int i = 1; i < rungs; i++)
		{
			float t = i / (float)rungs;
			m_Paint.Segment(ax + dx * t + nx, ay + dy * t + ny, ax + dx * t - nx, ay + dy * t - ny, cell * 0.05, 0xFFB07A3E);
		}

		m_Paint.Segment(ax + nx + 2, ay + ny + 3, bx + nx + 2, by + ny + 3, cell * 0.08, 0x55000000);
		m_Paint.Segment(ax + nx, ay + ny, bx + nx, by + ny, cell * 0.07, 0xFF7A4B1E);
		m_Paint.Segment(ax - nx, ay - ny, bx - nx, by - ny, cell * 0.07, 0xFF7A4B1E);
	}

	//------------------------------------------------------------------------------------------------
	protected void Snake(float hx, float hy, float tx, float ty, float cell)
	{
		float dx = tx - hx;
		float dy = ty - hy;
		float len = Math.Sqrt(dx * dx + dy * dy);
		if (len < 1)
			return;

		float px = -dy / len;
		float py = dx / len;
		array<float> body = {};
		array<float> shade = {};
		int seg = 28;
		for (int i = 0; i <= seg; i++)
		{
			float t = i / (float)seg;
			float wave = Math.Sin(t * Math.PI * 4) * cell * 0.24 * (1 - t * 0.4);
			body.Insert(hx + dx * t + px * wave);
			body.Insert(hy + dy * t + py * wave);
			shade.Insert(hx + dx * t + px * wave + 2);
			shade.Insert(hy + dy * t + py * wave + 3);
		}

		array<float> edge = {};
		edge.Copy(body);
		m_Paint.Line(shade, cell * 0.2, 0x44000000);
		m_Paint.Line(edge, cell * 0.2, 0xFF1D5E20);
		m_Paint.Line(body, cell * 0.13, 0xFF56B947);
		m_Paint.Disc(hx, hy, cell * 0.17, 0xFF56B947, 0xFF1D5E20, 2);
		m_Paint.Circle(hx - cell * 0.06, hy - cell * 0.04, cell * 0.045, 0xFFFFFFFF);
		m_Paint.Circle(hx + cell * 0.06, hy - cell * 0.04, cell * 0.045, 0xFFFFFFFF);
		m_Paint.Circle(hx - cell * 0.06, hy - cell * 0.04, cell * 0.022, 0xFF000000);
		m_Paint.Circle(hx + cell * 0.06, hy - cell * 0.04, cell * 0.022, 0xFF000000);
	}

	//------------------------------------------------------------------------------------------------
	override void Build(float x, float y, float w, float h)
	{
		float size = w;
		if (h < size)
			size = h;

		size -= 20;
		float cell = size / 10;
		float ox = x + (w - size) * 0.5;
		float oy = y + (h - size) * 0.5;
		m_Paint.Round(ox - 8, oy - 6, size + 16, size + 16, 12, 0x66000000);
		m_Paint.Round(ox - 10, oy - 10, size + 20, size + 20, 12, 0xFF3A2A1A);
		array<int> pal = {0xFFFFF3C4, 0xFFC9E8F5, 0xFFD8F0C8, 0xFFF8D3D3, 0xFFE6DBF5};
		for (int n = 1; n <= 100; n++)
		{
			float cx;
			float cy;
			Center(n, ox, oy, cell, cx, cy);
			int r = (n - 1) / 10;
			int c = (n - 1) % 10;
			m_Paint.Rect(cx - cell * 0.5, cy - cell * 0.5, cell, cell, pal[(r + c) % 5]);
			TextAt(cx - cell * 0.46, cy - cell * 0.48, cell * 0.6, 14, n.ToString(), 11, 0xFF6A5A48);
		}

		float hx;
		float hy;
		Center(100, ox, oy, cell, hx, hy);
		m_Paint.Ring(hx, hy, cell * 0.38, 3, 0xFFE3B230);
		array<int> jumps = IntList("jumps");
		for (int i = 0; i + 1 < jumps.Count(); i += 2)
		{
			float ax;
			float ay;
			float bx;
			float by;
			Center(jumps[i], ox, oy, cell, ax, ay);
			Center(jumps[i + 1], ox, oy, cell, bx, by);
			if (jumps[i + 1] > jumps[i])
				Ladder(ax, ay, bx, by, cell);
		}

		for (int i = 0; i + 1 < jumps.Count(); i += 2)
		{
			float ax;
			float ay;
			float bx;
			float by;
			Center(jumps[i], ox, oy, cell, ax, ay);
			Center(jumps[i + 1], ox, oy, cell, bx, by);
			if (jumps[i + 1] < jumps[i])
				Snake(ax, ay, bx, by, cell);
		}

		array<int> pos = IntList("pos");
		foreach (int s, int p : pos)
		{
			float cx;
			float cy;
			Center(p, ox, oy, cell, cx, cy);
			int sc = s % 2;
			int sr = s / 2;
			float tx = cx + (sc - 0.5) * cell * 0.4;
			float ty = cy + (sr - 0.3) * cell * 0.4;
			m_Paint.Circle(tx + 2, ty + 3, cell * 0.19, 0x66000000);
			m_Paint.Disc(tx, ty, cell * 0.19, SeatColor(s), 0xFF1A1A1A, 2);
			m_Paint.Circle(tx - cell * 0.05, ty - cell * 0.05, cell * 0.06, 0x88FFFFFF);
		}
	}

	//------------------------------------------------------------------------------------------------
	override float BuildSide(float x, float y, float w)
	{
		Die(x + (w - 90) * 0.5, y, 90, I("roll"));
		float cy = y + 104;
		if (MyTurn())
		{
			ButtonAt(x, cy, w, 40, "Roll the die", ID_CTRL, true);
			cy += 48;
		}

		array<int> pos = IntList("pos");
		foreach (int s, int p : pos)
		{
			m_Paint.Disc(x + 10, cy + 10, 8, SeatColor(s), 0xFF1A1A1A, 1.5);
			TextAt(x + 26, cy + 1, w - 26, 20, SeatName(s) + ": square " + p.ToString(), 14, 0xFFE0E0E0);
			cy += 24;
		}

		m_sHint = "";
		if (MyTurn())
			m_sHint = "Roll the die. Ladders lift you, snakes drop you; land exactly on 100.";

		return cy - y;
	}

	//------------------------------------------------------------------------------------------------
	override string Click(int id, out bool redraw)
	{
		redraw = false;
		if (id == ID_CTRL)
			return "roll";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void Preview(float x, float y, float w, float h)
	{
		float cell = h * 0.9;
		Ladder(x + w * 0.5 - cell * 0.9, y + h * 0.95, x + w * 0.5 - cell * 0.4, y + h * 0.05, cell);
		Snake(x + w * 0.5 + cell * 0.2, y + h * 0.15, x + w * 0.5 + cell * 1.1, y + h * 0.9, cell);
	}
}

//! Battleship: own fleet on the left, enemy waters on the right. Placement in the side panel, then click the
//! enemy grid to fire.
class ROOT_AER_GameBoardBattleship : ROOT_AER_GameBoard
{
	protected int m_iShip = 1;
	protected bool m_bVertical;

	//------------------------------------------------------------------------------------------------
	//! Column of a square.
	protected static int Cx(int sq)
	{
		return sq % 10;
	}

	//------------------------------------------------------------------------------------------------
	//! Row of a square from the top (rank 10 on top).
	protected static int Ry(int sq)
	{
		return 9 - sq / 10;
	}

	//------------------------------------------------------------------------------------------------
	protected static int Length(int ship)
	{
		array<int> l = {5, 4, 3, 3, 2};
		return l[ship - 1];
	}

	//------------------------------------------------------------------------------------------------
	protected static string ShipName(int ship)
	{
		array<string> n = {"Carrier", "Battleship", "Cruiser", "Submarine", "Destroyer"};
		return n[ship - 1];
	}

	//------------------------------------------------------------------------------------------------
	//! Grid with labels; returns the cell size. ox, oy: top left cell.
	protected float Grid(float x, float y, float w, float h, string title, int water, out float ox, out float oy)
	{
		float size = w - 26;
		if (h - 56 < size)
			size = h - 56;

		float cell = size / 10;
		ox = x + 22 + (w - 26 - size) * 0.5;
		oy = y + 46;
		TextMid(ox + size * 0.5, y + 12, size, title, 16, 0xFFFFFFFF);
		m_Paint.Round(ox - 4, oy - 2, size + 8, size + 8, 6, 0x66000000);
		m_Paint.Rect(ox, oy, size, size, water);
		for (int i = 0; i <= 10; i++)
		{
			m_Paint.Segment(ox + i * cell, oy, ox + i * cell, oy + size, 1, 0x40FFFFFF);
			m_Paint.Segment(ox, oy + i * cell, ox + size, oy + i * cell, 1, 0x40FFFFFF);
		}

		for (int i = 0; i < 10; i++)
		{
			TextMid(ox + i * cell + cell * 0.5, oy - 12, cell, (97 + i).AsciiToString(), 12, 0xFFB8C8D8);
			TextMid(ox - 12, oy + (9 - i) * cell + cell * 0.5, 22, (i + 1).ToString(), 12, 0xFFB8C8D8);
		}

		return cell;
	}

	//------------------------------------------------------------------------------------------------
	protected void Marker(float cx, float cy, float cell, int shot)
	{
		if (shot == 1)
		{
			m_Paint.Circle(cx, cy, cell * 0.12, 0xDDFFFFFF);
			return;
		}

		if (shot == 2)
		{
			m_Paint.Circle(cx, cy, cell * 0.36, 0x88FF6A00);
			m_Paint.Circle(cx, cy, cell * 0.26, 0xFFE8311A);
			m_Paint.Segment(cx - cell * 0.16, cy - cell * 0.16, cx + cell * 0.16, cy + cell * 0.16, cell * 0.07, 0xFFFFE0A0);
			m_Paint.Segment(cx - cell * 0.16, cy + cell * 0.16, cx + cell * 0.16, cy - cell * 0.16, cell * 0.07, 0xFFFFE0A0);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Build(float x, float y, float w, float h)
	{
		m_Paint.Round(x, y, w, h, 14, 0xFF0E2233);
		float gw = (w - 30) * 0.5;
		float ox;
		float oy;
		float cell = Grid(x + 10, y + 8, gw, h - 16, "Your fleet", 0xFF1B5E8C, ox, oy);
		array<int> ships = IntList("ships");
		array<int> hits = IntList("hits");
		array<int> shots = IntList("shots");
		array<int> sunk = IntList("sunk");
		bool placing = I("placing") == 1;
		bool ready = I("ready") == 1;
		if (ships.Count() >= 100)
		{
			for (int sq = 0; sq < 100; sq++)
			{
				int ship = ships[sq];
				if (ship == 0)
					continue;

				float cx = ox + Cx(sq) * cell;
				float cy = oy + Ry(sq) * cell;
				m_Paint.Round(cx + 3, cy + 3, cell - 6, cell - 6, cell * 0.2, 0xFF8D99A4);
				if (sq % 10 < 9 && ships[sq + 1] == ship)
					m_Paint.Rect(cx + cell * 0.5, cy + 3, cell, cell - 6, 0xFF8D99A4);

				if (sq < 90 && ships[sq + 10] == ship)
					m_Paint.Rect(cx + 3, cy - cell * 0.5, cell - 6, cell, 0xFF8D99A4);

				m_Paint.Circle(cx + cell * 0.5, cy + cell * 0.5, cell * 0.1, 0xFF5E6872);
			}
		}

		for (int sq = 0; sq < 100 && sq < hits.Count(); sq++)
		{
			Marker(ox + Cx(sq) * cell + cell * 0.5, oy + Ry(sq) * cell + cell * 0.5, cell, hits[sq]);
		}

		if (placing && !ready)
		{
			for (int sq = 0; sq < 100; sq++)
			{
				HitAt(ox + Cx(sq) * cell, oy + Ry(sq) * cell, cell, cell, ID_BASE + sq);
			}
		}

		float ex;
		float ey;
		float ecell = Grid(x + 20 + gw, y + 8, gw, h - 16, "Enemy waters", 0xFF123E5E, ex, ey);
		foreach (int sq : sunk)
		{
			m_Paint.Rect(ex + Cx(sq) * ecell + 1, ey + Ry(sq) * ecell + 1, ecell - 2, ecell - 2, 0xFF5A1E1E);
		}

		for (int sq = 0; sq < 100 && sq < shots.Count(); sq++)
		{
			Marker(ex + Cx(sq) * ecell + ecell * 0.5, ey + Ry(sq) * ecell + ecell * 0.5, ecell, shots[sq]);
		}

		if (!placing && MyTurn())
		{
			for (int sq = 0; sq < 100 && sq < shots.Count(); sq++)
			{
				if (shots[sq] == 0)
					HitAt(ex + Cx(sq) * ecell, ey + Ry(sq) * ecell, ecell, ecell, ID_BASE + 100 + sq);
			}
		}

		if (placing)
		{
			string wait = "The enemy is placing ships";
			if (I("oppready") == 1)
				wait = "The enemy fleet is ready";

			TextMid(ex + ecell * 5, ey + ecell * 5, ecell * 10, wait, 15, 0xFFB8C8D8);
		}
	}

	//------------------------------------------------------------------------------------------------
	override float BuildSide(float x, float y, float w)
	{
		bool placing = I("placing") == 1;
		bool ready = I("ready") == 1;
		m_sHint = "";
		if (!placing)
		{
			if (MyTurn())
				m_sHint = "Click a square in enemy waters to fire. A hit fires again.";

			return 0;
		}

		if (ready)
		{
			m_sHint = "Fleet ready. Waiting for the other player.";
			return 0;
		}

		array<int> ships = IntList("ships");
		float cy = y;
		TextAt(x, cy, w, 20, "Choose a ship, then click your grid:", 14, 0xFFE0E0E0);
		cy += 26;
		for (int s = 1; s <= 5; s++)
		{
			int bg = 0x00000000;
			if (s == m_iShip)
				bg = 0x553D7BE0;

			m_Paint.Rect(x, cy, w, 30, bg);
			bool placed = ships.Contains(s);
			for (int k = 0; k < Length(s); k++)
			{
				int col = 0xFF6D7884;
				if (placed)
					col = 0xFF9AC48A;

				m_Paint.Round(x + 6 + k * 15, cy + 9, 13, 13, 3, col);
			}

			TextAt(x + 90, cy + 6, w - 90, 20, ShipName(s), 14, 0xFFFFFFFF);
			HitAt(x, cy, w, 30, ID_CTRL + s);
			cy += 32;
		}

		cy += 6;
		string dir = "Direction: across (right)";
		if (m_bVertical)
			dir = "Direction: up";

		ButtonAt(x, cy, w, 30, dir, ID_CTRL + 10);
		cy += 36;
		float bw = (w - 8) * 0.5;
		ButtonAt(x, cy, bw, 30, "Random", ID_CTRL + 11);
		ButtonAt(x + bw + 8, cy, bw, 30, "Clear", ID_CTRL + 12);
		cy += 36;
		ButtonAt(x, cy, w, 36, "Ready", ID_CTRL + 13, true);
		cy += 42;
		m_sHint = "A ship starts on the square you click and runs to the right or up.";
		return cy - y;
	}

	//------------------------------------------------------------------------------------------------
	override string Click(int id, out bool redraw)
	{
		redraw = true;
		if (id > ID_CTRL && id <= ID_CTRL + 5)
		{
			m_iShip = id - ID_CTRL;
			return "";
		}

		switch (id)
		{
			case ID_CTRL + 10:
			{
				m_bVertical = !m_bVertical;
				return "";
			}

			case ID_CTRL + 11: return "random";
			case ID_CTRL + 12: return "clear";
			case ID_CTRL + 13: return "ready";
		}

		int sq = id - ID_BASE;
		if (sq >= 0 && sq < 100)
		{
			string dir = "h";
			if (m_bVertical)
				dir = "v";

			string move = string.Format("place %1 %2 %3", m_iShip, ROOT_AER_GameRules.SquareName(sq, 10), dir);
			if (m_iShip < 5)
				m_iShip++;

			return move;
		}

		if (sq >= 100 && sq < 200)
			return ROOT_AER_GameRules.SquareName(sq - 100, 10);

		redraw = false;
		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void Preview(float x, float y, float w, float h)
	{
		float cell = h * 0.3;
		float ox = x + w * 0.5 - cell * 2.5;
		float oy = y + h * 0.05;
		m_Paint.Rect(ox, oy, cell * 5, cell * 3, 0xFF1B5E8C);
		m_Paint.Round(ox + cell * 0.6, oy + cell + 4, cell * 3.2, cell - 8, cell * 0.4, 0xFF8D99A4);
		Marker(ox + cell * 4.3, oy + cell * 0.5, cell, 2);
		Marker(ox + cell * 1.5, oy + cell * 2.5, cell, 1);
	}
}
