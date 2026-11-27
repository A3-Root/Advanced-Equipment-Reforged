//! Chess board: click a piece, then a highlighted square. Promotion asks for the piece in the side panel.
class ROOT_AER_GameBoardChess : ROOT_AER_GameBoard
{
	protected ref array<int> m_aBoard = {};
	protected ref array<int> m_aMoves = {};
	protected bool m_bWhite;
	protected int m_iSel = -1;
	protected int m_iPromo = -1;		//!< from * 64 + to waiting for the promotion piece

	//------------------------------------------------------------------------------------------------
	protected override void OnNewState()
	{
		m_aBoard = IntList("board");
		m_aMoves = IntList("moves");
		m_bWhite = I("white") == 1;
		m_iSel = -1;
		m_iPromo = -1;
	}

	//------------------------------------------------------------------------------------------------
	protected int Col(int sq)
	{
		if (m_bWhite)
			return sq % 8;

		return 7 - sq % 8;
	}

	//------------------------------------------------------------------------------------------------
	protected int Row(int sq)
	{
		if (m_bWhite)
			return 7 - sq / 8;

		return sq / 8;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Own(int piece)
	{
		if (m_bWhite)
			return piece > 0;

		return piece < 0;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FromHas(int sq)
	{
		foreach (int m : m_aMoves)
		{
			if (m / 64 == sq)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Wooden frame, squares, last move, selection, coordinates. Returns the cell size; ox, oy: top left square.
	protected float Squares(float x, float y, float w, float h, out float ox, out float oy, int light, int dark)
	{
		float rim = 22;
		float size = w;
		if (h < size)
			size = h;

		size -= rim * 2;
		float cell = size / 8;
		ox = x + (w - size) * 0.5;
		oy = y + (h - size) * 0.5;
		m_Paint.Round(ox - rim + 4, oy - rim + 6, size + rim * 2, size + rim * 2, 10, 0x66000000);
		m_Paint.Round(ox - rim, oy - rim, size + rim * 2, size + rim * 2, 10, 0xFF4A2C17);
		m_Paint.Rect(ox - 3, oy - 3, size + 6, size + 6, 0xFF2A180C);
		for (int sq = 0; sq < 64; sq++)
		{
			int col = light;
			if ((sq / 8 + sq % 8) % 2 == 0)
				col = dark;

			m_Paint.Rect(ox + Col(sq) * cell, oy + Row(sq) * cell, cell, cell, col);
		}

		for (int i = 0; i < 8; i++)
		{
			string file = (97 + i).AsciiToString();
			string rank = (i + 1).ToString();
			float fx = i;
			float ry = 7 - i;
			if (!m_bWhite)
			{
				fx = 7 - i;
				ry = i;
			}

			TextMid(ox + fx * cell + cell * 0.5, oy + size + rim * 0.5, cell, file, 13, 0xFFE8D7BE);
			TextMid(ox - rim * 0.5, oy + ry * cell + cell * 0.5, rim, rank, 13, 0xFFE8D7BE);
		}

		return cell;
	}

	//------------------------------------------------------------------------------------------------
	protected void Tint(int sq, float ox, float oy, float cell, int color)
	{
		m_Paint.Rect(ox + Col(sq) * cell, oy + Row(sq) * cell, cell, cell, color);
	}

	//------------------------------------------------------------------------------------------------
	//! Last move squares and selection.
	protected void Marks(string last, float ox, float oy, float cell)
	{
		if (last.Length() >= 4)
		{
			int a = ROOT_AER_GameRules.Square(last.Substring(0, 2));
			int b = ROOT_AER_GameRules.Square(last.Substring(2, 2));
			if (a >= 0 && b >= 0)
			{
				Tint(a, ox, oy, cell, 0x70D8D43C);
				Tint(b, ox, oy, cell, 0x70D8D43C);
			}
		}

		if (m_iSel >= 0)
			Tint(m_iSel, ox, oy, cell, 0x9060A8E8);
	}

	//------------------------------------------------------------------------------------------------
	//! Dots on free targets of the selected piece, rings on captures.
	protected void Targets(float ox, float oy, float cell)
	{
		if (m_iSel < 0)
			return;

		foreach (int m : m_aMoves)
		{
			if (m / 64 != m_iSel)
				continue;

			int to = m % 64;
			float cx = ox + Col(to) * cell + cell * 0.5;
			float cy = oy + Row(to) * cell + cell * 0.5;
			if (m_aBoard[to] == 0)
				m_Paint.Circle(cx, cy, cell * 0.15, 0x66101010);
			else
				m_Paint.Ring(cx, cy, cell * 0.44, cell * 0.08, 0x77101010);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Build(float x, float y, float w, float h)
	{
		float ox;
		float oy;
		float cell = Squares(x, y, w, h, ox, oy, 0xFFF0D9B5, 0xFFB58863);
		Marks(S("last"), ox, oy, cell);
		int check = I("check", -1);
		if (check >= 0)
		{
			float kx = ox + Col(check) * cell + cell * 0.5;
			float ky = oy + Row(check) * cell + cell * 0.5;
			m_Paint.Circle(kx, ky, cell * 0.48, 0x66FF2020);
			m_Paint.Circle(kx, ky, cell * 0.34, 0x88FF2020);
		}

		for (int sq = 0; sq < 64; sq++)
		{
			int p = m_aBoard[sq];
			if (p != 0)
				Piece(m_Paint, Math.AbsInt(p), p > 0, ox + Col(sq) * cell, oy + Row(sq) * cell, cell);
		}

		Targets(ox, oy, cell);
		if (!MyTurn() || m_iPromo >= 0)
			return;

		for (int sq = 0; sq < 64; sq++)
		{
			bool target = m_iSel >= 0 && m_aMoves.Contains(m_iSel * 64 + sq);
			if (target || (Own(m_aBoard[sq]) && FromHas(sq)))
				HitAt(ox + Col(sq) * cell, oy + Row(sq) * cell, cell, cell, ID_BASE + sq);
		}
	}

	//------------------------------------------------------------------------------------------------
	override float BuildSide(float x, float y, float w)
	{
		float cy = y;
		string side = "Black";
		if (m_bWhite)
			side = "White";

		TextAt(x, cy, w, 22, "You play " + side, 17, 0xFFFFFFFF);
		cy += 28;
		if (m_iPromo >= 0)
		{
			TextAt(x, cy, w, 20, "Promote the pawn to:", 14, 0xFFE8D7BE);
			cy += 26;
			array<string> names = {"Queen", "Rook", "Bishop", "Knight"};
			array<int> types = {5, 4, 3, 2};
			float bw = (w - 6) * 0.5;
			foreach (int i, string n : names)
			{
				int pc = i % 2;
				int pr = i / 2;
				float bx = x + pc * (bw + 6);
				float by = cy + pr * 64;
				ButtonAt(bx, by, bw, 58, "", ID_CTRL + i);
				Piece(m_Paint, types[i], m_bWhite, bx + 4, by + 4, 50);
				TextAt(bx + 54, by + 18, bw - 58, 20, n, 14, 0xFFFFFFFF);
			}

			ButtonAt(x, cy + 132, w, 28, "Cancel", ID_CTRL + 4);
			return cy + 168 - y;
		}

		// Captured pieces: missing from the starting set.
		array<int> start = {0, 8, 2, 2, 2, 1, 0};
		for (int colour = 0; colour < 2; colour++)
		{
			bool whitePieces = colour == 0;
			string label = "Taken from black:";
			if (whitePieces)
				label = "Taken from white:";

			TextAt(x, cy, w, 18, label, 13, 0xFFB8B8B8);
			cy += 20;
			float px = x;
			for (int t = 5; t >= 1; t--)
			{
				int count;
				foreach (int p : m_aBoard)
				{
					if ((whitePieces && p == t) || (!whitePieces && p == -t))
						count++;
				}

				for (int k = count; k < start[t]; k++)
				{
					if (px > x + w - 22)
					{
						px = x;
						cy += 26;
					}

					Piece(m_Paint, t, whitePieces, px, cy, 26);
					px += 18;
				}
			}

			cy += 32;
		}

		if (MyTurn())
			m_sHint = "Click one of your pieces, then a marked square.";
		else
			m_sHint = "";

		return cy - y;
	}

	//------------------------------------------------------------------------------------------------
	override string Click(int id, out bool redraw)
	{
		redraw = true;
		if (m_iPromo >= 0 && id >= ID_CTRL && id <= ID_CTRL + 4)
		{
			int m = m_iPromo;
			m_iPromo = -1;
			m_iSel = -1;
			if (id == ID_CTRL + 4)
				return "";

			array<string> letters = {"q", "r", "b", "n"};
			return ROOT_AER_GameRules.SquareName(m / 64) + ROOT_AER_GameRules.SquareName(m % 64) + letters[id - ID_CTRL];
		}

		int sq = id - ID_BASE;
		if (sq < 0 || sq >= 64)
		{
			redraw = false;
			return "";
		}

		if (m_iSel >= 0 && m_aMoves.Contains(m_iSel * 64 + sq))
		{
			int from = m_iSel;
			int rank = sq / 8;
			if (Math.AbsInt(m_aBoard[from]) == 1 && (rank == 0 || rank == 7))
			{
				m_iPromo = from * 64 + sq;
				return "";
			}

			m_iSel = -1;
			return ROOT_AER_GameRules.SquareName(from) + ROOT_AER_GameRules.SquareName(sq);
		}

		if (sq == m_iSel || !Own(m_aBoard[sq]))
			m_iSel = -1;
		else
			m_iSel = sq;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void Preview(float x, float y, float w, float h)
	{
		float s = h * 0.8;
		m_Paint.Ellipse(x + w * 0.5, y + h * 0.88, w * 0.4, h * 0.06, 0x40000000);
		Piece(m_Paint, 6, true, x + w * 0.5 - s * 0.85, y + h * 0.1, s);
		Piece(m_Paint, 2, false, x + w * 0.5 - s * 0.2, y + h * 0.15, s * 0.95);
		Piece(m_Paint, 1, true, x + w * 0.5 + s * 0.35, y + h * 0.3, s * 0.75);
	}

	//------------------------------------------------------------------------------------------------
	//! Piece type 1 pawn, 2 knight, 3 bishop, 4 rook, 5 queen, 6 king in the square box x, y, s.
	static void Piece(ROOT_AER_GamePaint p, int type, bool white, float x, float y, float s)
	{
		int fill = 0xFF2E2B28;
		int edge = 0xFF8C8C8C;
		if (white)
		{
			fill = 0xFFF8F3E6;
			edge = 0xFF2A2A2A;
		}

		p.Ellipse(x + s * 0.5, y + s * 0.88, s * 0.3, s * 0.05, 0x45000000);
		array<float> foot = {0.22, 0.9, 0.78, 0.9, 0.78, 0.82, 0.22, 0.82};
		array<float> base = {0.28, 0.82, 0.72, 0.82, 0.66, 0.74, 0.34, 0.74};
		p.Shape(x, y, s, foot, fill, edge);
		p.Shape(x, y, s, base, fill, edge);
		switch (type)
		{
			case 1:
			{
				array<float> body = {0.38, 0.74, 0.62, 0.74, 0.56, 0.47, 0.44, 0.47};
				p.Shape(x, y, s, body, fill, edge);
				p.UnitEllipse(x, y, s, 0.5, 0.47, 0.13, 0.04, fill, edge);
				p.UnitEllipse(x, y, s, 0.5, 0.33, 0.13, 0.13, fill, edge);
				break;
			}

			case 2:
			{
				array<float> neck = {0.34, 0.74, 0.7, 0.74, 0.66, 0.36, 0.46, 0.24};
				array<float> snout = {0.2, 0.44, 0.44, 0.26, 0.54, 0.4, 0.27, 0.54};
				array<float> head = {0.42, 0.26, 0.6, 0.15, 0.7, 0.32, 0.54, 0.42};
				array<float> ear = {0.52, 0.2, 0.58, 0.06, 0.65, 0.19};
				p.Shape(x, y, s, neck, fill, edge);
				p.Shape(x, y, s, snout, fill, edge);
				p.Shape(x, y, s, head, fill, edge);
				p.Shape(x, y, s, ear, fill, edge);
				p.Circle(x + s * 0.47, y + s * 0.3, s * 0.03, edge);
				break;
			}

			case 3:
			{
				array<float> body = {0.38, 0.74, 0.62, 0.74, 0.56, 0.5, 0.44, 0.5};
				p.Shape(x, y, s, body, fill, edge);
				p.UnitEllipse(x, y, s, 0.5, 0.5, 0.14, 0.04, fill, edge);
				p.UnitEllipse(x, y, s, 0.5, 0.35, 0.12, 0.16, fill, edge);
				p.UnitEllipse(x, y, s, 0.5, 0.15, 0.05, 0.05, fill, edge);
				p.Segment(x + s * 0.5, y + s * 0.3, x + s * 0.58, y + s * 0.4, s * 0.025, edge);
				break;
			}

			case 4:
			{
				array<float> body = {0.33, 0.74, 0.67, 0.74, 0.64, 0.36, 0.36, 0.36};
				array<float> top = {0.28, 0.36, 0.72, 0.36, 0.72, 0.25, 0.28, 0.25};
				array<float> c1 = {0.28, 0.25, 0.38, 0.25, 0.38, 0.15, 0.28, 0.15};
				array<float> c2 = {0.45, 0.25, 0.55, 0.25, 0.55, 0.15, 0.45, 0.15};
				array<float> c3 = {0.62, 0.25, 0.72, 0.25, 0.72, 0.15, 0.62, 0.15};
				p.Shape(x, y, s, body, fill, edge);
				p.Shape(x, y, s, top, fill, edge);
				p.Shape(x, y, s, c1, fill, edge);
				p.Shape(x, y, s, c2, fill, edge);
				p.Shape(x, y, s, c3, fill, edge);
				break;
			}

			case 5:
			{
				array<float> body = {0.34, 0.74, 0.66, 0.74, 0.58, 0.42, 0.42, 0.42};
				array<float> crown = {0.36, 0.42, 0.64, 0.42, 0.74, 0.2, 0.26, 0.2};
				p.Shape(x, y, s, body, fill, edge);
				p.Shape(x, y, s, crown, fill, edge);
				p.UnitEllipse(x, y, s, 0.5, 0.42, 0.17, 0.045, fill, edge);
				array<float> balls = {0.26, 0.19, 0.38, 0.15, 0.5, 0.12, 0.62, 0.15, 0.74, 0.19};
				for (int i = 0; i + 1 < balls.Count(); i += 2)
				{
					p.UnitEllipse(x, y, s, balls[i], balls[i + 1], 0.045, 0.045, fill, edge);
				}

				break;
			}

			case 6:
			{
				array<float> body = {0.34, 0.74, 0.66, 0.74, 0.58, 0.42, 0.42, 0.42};
				array<float> dome = {0.34, 0.42, 0.66, 0.42, 0.7, 0.27, 0.3, 0.27};
				array<float> upright = {0.465, 0.27, 0.535, 0.27, 0.535, 0.05, 0.465, 0.05};
				array<float> arm = {0.4, 0.11, 0.6, 0.11, 0.6, 0.175, 0.4, 0.175};
				p.Shape(x, y, s, body, fill, edge);
				p.Shape(x, y, s, dome, fill, edge);
				p.UnitEllipse(x, y, s, 0.5, 0.42, 0.17, 0.045, fill, edge);
				p.Shape(x, y, s, upright, fill, edge);
				p.Shape(x, y, s, arm, fill, edge);
				break;
			}
		}
	}
}

//! Checkers board: click a piece, then a highlighted square; a capture chain continues with the same piece.
class ROOT_AER_GameBoardCheckers : ROOT_AER_GameBoardChess
{
	//------------------------------------------------------------------------------------------------
	protected override void OnNewState()
	{
		super.OnNewState();
		int chain = I("chain", -1);
		if (chain >= 0 && MyTurn())
			m_iSel = chain;
	}

	//------------------------------------------------------------------------------------------------
	override void Build(float x, float y, float w, float h)
	{
		float ox;
		float oy;
		float cell = Squares(x, y, w, h, ox, oy, 0xFFEBD9B4, 0xFF5C3A21);
		Marks(S("last"), ox, oy, cell);
		for (int sq = 0; sq < 64; sq++)
		{
			int p = m_aBoard[sq];
			if (p != 0)
				Man(m_Paint, p > 0, Math.AbsInt(p) == 2, ox + Col(sq) * cell + cell * 0.5, oy + Row(sq) * cell + cell * 0.5, cell * 0.4);
		}

		Targets(ox, oy, cell);
		if (!MyTurn())
			return;

		for (int sq = 0; sq < 64; sq++)
		{
			bool target = m_iSel >= 0 && m_aMoves.Contains(m_iSel * 64 + sq);
			if (target || (Own(m_aBoard[sq]) && FromHas(sq)))
				HitAt(ox + Col(sq) * cell, oy + Row(sq) * cell, cell, cell, ID_BASE + sq);
		}
	}

	//------------------------------------------------------------------------------------------------
	override float BuildSide(float x, float y, float w)
	{
		string side = "Black";
		if (m_bWhite)
			side = "White";

		TextAt(x, y, w, 22, "You play " + side, 17, 0xFFFFFFFF);
		float cy = y + 30;
		for (int colour = 0; colour < 2; colour++)
		{
			bool whiteMen = colour == 0;
			int count;
			foreach (int p : m_aBoard)
			{
				if ((whiteMen && p > 0) || (!whiteMen && p < 0))
					count++;
			}

			Man(m_Paint, whiteMen, false, x + 14, cy + 14, 12);
			string label = "Black: ";
			if (whiteMen)
				label = "White: ";

			TextAt(x + 34, cy + 4, w - 34, 20, label + count.ToString() + " pieces", 14, 0xFFE0E0E0);
			cy += 32;
		}

		m_sHint = "";
		if (MyTurn())
		{
			m_sHint = "Click one of your pieces, then a marked square. Captures are compulsory.";
			if (I("chain", -1) >= 0)
				m_sHint = "Keep capturing with the marked piece.";
		}

		return cy - y;
	}

	//------------------------------------------------------------------------------------------------
	override string Click(int id, out bool redraw)
	{
		redraw = true;
		int sq = id - ID_BASE;
		if (sq < 0 || sq >= 64)
		{
			redraw = false;
			return "";
		}

		if (m_iSel >= 0 && m_aMoves.Contains(m_iSel * 64 + sq))
		{
			int from = m_iSel;
			m_iSel = -1;
			return ROOT_AER_GameRules.SquareName(from) + ROOT_AER_GameRules.SquareName(sq);
		}

		if (sq == m_iSel || !Own(m_aBoard[sq]) || I("chain", -1) >= 0)
			m_iSel = I("chain", -1);
		else
			m_iSel = sq;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	override void Preview(float x, float y, float w, float h)
	{
		float r = h * 0.26;
		Man(m_Paint, false, false, x + w * 0.5 - r * 1.1, y + h * 0.58, r);
		Man(m_Paint, true, true, x + w * 0.5 + r * 0.9, y + h * 0.45, r);
	}

	//------------------------------------------------------------------------------------------------
	//! Disc centred on cx, cy; king with a crown.
	static void Man(ROOT_AER_GamePaint p, bool white, bool king, float cx, float cy, float r)
	{
		int fill = 0xFF2B2B2B;
		int ring = 0xFF4A4A4A;
		int edge = 0xFF0A0A0A;
		if (white)
		{
			fill = 0xFFF2E8D5;
			ring = 0xFFD1C2A5;
			edge = 0xFF7A6A50;
		}

		p.Circle(cx + r * 0.08, cy + r * 0.14, r, 0x66000000);
		p.Disc(cx, cy, r, fill, edge, r * 0.08);
		p.Ring(cx, cy, r * 0.68, r * 0.1, ring);
		p.Ring(cx, cy, r * 0.4, r * 0.06, ring);
		if (!king)
			return;

		int gold = 0xFFE3B230;
		array<float> band = {cx - r * 0.4, cy + r * 0.25, cx + r * 0.4, cy + r * 0.25, cx + r * 0.4, cy + r * 0.05, cx - r * 0.4, cy + r * 0.05};
		p.Poly(band, gold);
		p.Tri(cx - r * 0.4, cy + r * 0.06, cx - r * 0.2, cy + r * 0.06, cx - r * 0.42, cy - r * 0.32, gold);
		p.Tri(cx - r * 0.13, cy + r * 0.06, cx + r * 0.13, cy + r * 0.06, cx, cy - r * 0.42, gold);
		p.Tri(cx + r * 0.2, cy + r * 0.06, cx + r * 0.4, cy + r * 0.06, cx + r * 0.42, cy - r * 0.32, gold);
	}
}
