//! Chess with full legal move generation: castling, en passant, promotion, check, checkmate, stalemate.
//! Board index = file + rank * 8; white pieces positive (1 P, 2 N, 3 B, 4 R, 5 Q, 6 K), black negative.
class ROOT_AE3_GameChess : ROOT_AE3_GameRules
{
	protected ref array<int> m_aBoard = {};
	protected bool m_bWhiteToMove = true;
	protected int m_iCastle = 15;		//!< 1 white short, 2 white long, 4 black short, 8 black long
	protected int m_iEnPassant = -1;
	protected int m_iWhiteSeat;
	protected int m_iHalfMoves;
	protected string m_sLastMove;
	ref array<int> m_aSidePref = {0, 0};	//!< per seat: 0 random, 1 white, 2 black

	//------------------------------------------------------------------------------------------------
	override string GetId()
	{
		return "chess";
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Chess";
	}

	//------------------------------------------------------------------------------------------------
	override string GetHelp()
	{
		return "Write a move as two squares, for example e2e4. Promotion: e7e8q (q, r, b, n). Castle by moving the king two squares.";
	}

	//------------------------------------------------------------------------------------------------
	override void Start(int seats, int starter)
	{
		super.Start(seats, starter);
		m_iWhiteSeat = starter;
		if (m_aSidePref[0] == 1 || m_aSidePref[1] == 2)
			m_iWhiteSeat = 0;
		else if (m_aSidePref[0] == 2 || m_aSidePref[1] == 1)
			m_iWhiteSeat = 1;

		m_iTurn = m_iWhiteSeat;
		array<int> back = {4, 2, 3, 5, 6, 3, 2, 4};
		m_aBoard.Clear();
		m_aBoard.Resize(64);
		for (int f = 0; f < 8; f++)
		{
			m_aBoard[f] = back[f];
			m_aBoard[8 + f] = 1;
			m_aBoard[48 + f] = -1;
			m_aBoard[56 + f] = -back[f];
		}

		m_bWhiteToMove = true;
		m_iCastle = 15;
		m_iEnPassant = -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Own(int piece, bool white)
	{
		if (white)
			return piece > 0;

		return piece < 0;
	}

	//------------------------------------------------------------------------------------------------
	//! True when square sq is attacked by the given side on board b.
	protected static bool Attacked(array<int> b, int sq, bool byWhite)
	{
		int r = sq / 8;
		int f = sq % 8;
		int sign = -1;
		if (byWhite)
			sign = 1;

		// pawns
		int pr = r - sign;
		if (pr >= 0 && pr < 8)
		{
			if (f > 0 && b[pr * 8 + f - 1] == sign)
				return true;

			if (f < 7 && b[pr * 8 + f + 1] == sign)
				return true;
		}

		// knights
		array<int> kn = {1, 2, 2, 1, -1, 2, -2, 1, 1, -2, 2, -1, -1, -2, -2, -1};
		for (int i = 0; i < 16; i += 2)
		{
			int nr = r + kn[i];
			int nf = f + kn[i + 1];
			if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8 && b[nr * 8 + nf] == 2 * sign)
				return true;
		}

		// sliders and king
		array<int> dirs = {1, 0, -1, 0, 0, 1, 0, -1, 1, 1, 1, -1, -1, 1, -1, -1};
		for (int d = 0; d < 16; d += 2)
		{
			bool diag = d >= 8;
			int nr = r + dirs[d];
			int nf = f + dirs[d + 1];
			int dist = 1;
			while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
			{
				int p = b[nr * 8 + nf];
				if (p != 0)
				{
					if (p * sign > 0)
					{
						int t = p * sign;
						if (t == 5 || (diag && t == 3) || (!diag && t == 4) || (dist == 1 && t == 6))
							return true;
					}

					break;
				}

				nr += dirs[d];
				nf += dirs[d + 1];
				dist++;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static int KingSquare(array<int> b, bool white)
	{
		int k = -6;
		if (white)
			k = 6;

		return b.Find(k);
	}

	//------------------------------------------------------------------------------------------------
	//! Pseudo-legal moves as from * 64 + to.
	protected void Pseudo(array<int> b, bool white, array<int> moves)
	{
		int sign = -1;
		if (white)
			sign = 1;

		for (int sq = 0; sq < 64; sq++)
		{
			int p = b[sq];
			if (!Own(p, white))
				continue;

			int t = p * sign;
			int r = sq / 8;
			int f = sq % 8;
			if (t == 1)
			{
				int fwd = sq + 8 * sign;
				if (fwd >= 0 && fwd < 64 && b[fwd] == 0)
				{
					moves.Insert(sq * 64 + fwd);
					int startRank = 1;
					if (!white)
						startRank = 6;

					int two = sq + 16 * sign;
					if (r == startRank && b[two] == 0)
						moves.Insert(sq * 64 + two);
				}

				for (int df = -1; df <= 1; df += 2)
				{
					int nf = f + df;
					int nr = r + sign;
					if (nf < 0 || nf > 7 || nr < 0 || nr > 7)
						continue;

					int to = nr * 8 + nf;
					if ((b[to] != 0 && !Own(b[to], white)) || to == m_iEnPassant)
						moves.Insert(sq * 64 + to);
				}

				continue;
			}

			if (t == 2)
			{
				array<int> kn = {1, 2, 2, 1, -1, 2, -2, 1, 1, -2, 2, -1, -1, -2, -2, -1};
				for (int i = 0; i < 16; i += 2)
				{
					int nr = r + kn[i];
					int nf = f + kn[i + 1];
					if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8 && !Own(b[nr * 8 + nf], white))
						moves.Insert(sq * 64 + nr * 8 + nf);
				}

				continue;
			}

			array<int> dirs = {1, 0, -1, 0, 0, 1, 0, -1, 1, 1, 1, -1, -1, 1, -1, -1};
			int dStart = 0;
			int dEnd = 16;
			if (t == 3)
				dStart = 8;
			else if (t == 4)
				dEnd = 8;

			bool single = t == 6;
			for (int d = dStart; d < dEnd; d += 2)
			{
				int nr = r + dirs[d];
				int nf = f + dirs[d + 1];
				while (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
				{
					int to = nr * 8 + nf;
					if (Own(b[to], white))
						break;

					moves.Insert(sq * 64 + to);
					if (b[to] != 0 || single)
						break;

					nr += dirs[d];
					nf += dirs[d + 1];
				}
			}

			if (t == 6)
			{
				int home = 4;
				int shortBit = 1;
				int longBit = 2;
				if (!white)
				{
					home = 60;
					shortBit = 4;
					longBit = 8;
				}

				if (sq == home && !Attacked(b, home, !white))
				{
					if ((m_iCastle & shortBit) && b[home + 1] == 0 && b[home + 2] == 0 && b[home + 3] == 4 * sign && !Attacked(b, home + 1, !white))
						moves.Insert(sq * 64 + home + 2);

					if ((m_iCastle & longBit) && b[home - 1] == 0 && b[home - 2] == 0 && b[home - 3] == 0 && b[home - 4] == 4 * sign && !Attacked(b, home - 1, !white))
						moves.Insert(sq * 64 + home - 2);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Board after a move (promotion piece type promo, default queen).
	protected array<int> After(array<int> b, int from, int to, int promo)
	{
		array<int> n = {};
		n.Copy(b);
		int p = n[from];
		int t = Math.AbsInt(p);
		if (t == 1 && to == m_iEnPassant && n[to] == 0)
		{
			int capSq = to - 8;
			if (p < 0)
				capSq = to + 8;

			n[capSq] = 0;
		}

		if (t == 6 && Math.AbsInt(to - from) == 2)
		{
			if (to > from)
			{
				n[from + 1] = n[from + 3];
				n[from + 3] = 0;
			}
			else
			{
				n[from - 1] = n[from - 4];
				n[from - 4] = 0;
			}
		}

		n[to] = p;
		n[from] = 0;
		int rank = to / 8;
		if (t == 1 && (rank == 7 || rank == 0))
		{
			if (promo < 2 || promo > 5)
				promo = 5;

			if (p > 0)
				n[to] = promo;
			else
				n[to] = -promo;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected void Legal(bool white, array<int> legal)
	{
		array<int> pseudo = {};
		Pseudo(m_aBoard, white, pseudo);
		foreach (int m : pseudo)
		{
			array<int> n = After(m_aBoard, m / 64, m % 64, 5);
			if (!Attacked(n, KingSquare(n, white), !white))
				legal.Insert(m);
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

		move.ToLower();
		move = move.Trim();
		move.Replace("-", "");
		if (move.Length() < 4)
		{
			status = "Write a move as two squares, for example e2e4.";
			return false;
		}

		int from = Square(move.Substring(0, 2));
		int to = Square(move.Substring(2, 2));
		if (from < 0 || to < 0)
		{
			status = "Write a move as two squares, for example e2e4.";
			return false;
		}

		int promo = 5;
		if (move.Length() >= 5)
		{
			switch (move.Get(4))
			{
				case "n": promo = 2; break;
				case "b": promo = 3; break;
				case "r": promo = 4; break;
			}
		}

		array<int> pseudo = {};
		Pseudo(m_aBoard, m_bWhiteToMove, pseudo);
		if (!pseudo.Contains(from * 64 + to))
		{
			status = "That move is not legal.";
			return false;
		}

		array<int> n = After(m_aBoard, from, to, promo);
		if (Attacked(n, KingSquare(n, m_bWhiteToMove), !m_bWhiteToMove))
		{
			status = "That move would leave your king in check.";
			return false;
		}

		int moved = Math.AbsInt(m_aBoard[from]);
		bool capture = m_aBoard[to] != 0 || (moved == 1 && to == m_iEnPassant);
		m_aBoard = n;
		m_iEnPassant = -1;
		if (moved == 1 && Math.AbsInt(to - from) == 16)
			m_iEnPassant = (from + to) / 2;

		if (from == 4 || to == 4)
			m_iCastle &= ~3;

		if (from == 60 || to == 60)
			m_iCastle &= ~12;

		if (from == 7 || to == 7)
			m_iCastle &= ~1;

		if (from == 0 || to == 0)
			m_iCastle &= ~2;

		if (from == 63 || to == 63)
			m_iCastle &= ~4;

		if (from == 56 || to == 56)
			m_iCastle &= ~8;

		if (moved == 1 || capture)
			m_iHalfMoves = 0;
		else
			m_iHalfMoves++;

		m_sLastMove = SquareName(from) + SquareName(to);
		m_bWhiteToMove = !m_bWhiteToMove;
		NextTurn();
		status = "Move played.";
		array<int> replies = {};
		Legal(m_bWhiteToMove, replies);
		if (replies.IsEmpty())
		{
			if (Attacked(m_aBoard, KingSquare(m_aBoard, m_bWhiteToMove), !m_bWhiteToMove))
			{
				m_iWinner = seat;
				status = "Checkmate.";
			}
			else
			{
				m_iWinner = -2;
				status = "Stalemate - the game is a draw.";
			}
		}
		else if (m_iHalfMoves >= 100)
		{
			m_iWinner = -2;
			status = "Fifty moves without capture or pawn move - the game is a draw.";
		}

		m_sStatus = status;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Legal moves scored by captured material, promotion, check and whether the moved piece can be taken.
	override void BotMoves(int seat, notnull array<string> moves)
	{
		if (m_iWinner != -1 || seat != m_iTurn)
			return;

		array<int> legal = {};
		Legal(m_bWhiteToMove, legal);
		array<int> values = {0, 1, 3, 3, 5, 9, 0};
		array<string> cands = {};
		array<float> scores = {};
		foreach (int m : legal)
		{
			int from = m / 64;
			int to = m % 64;
			array<int> n = After(m_aBoard, from, to, 5);
			float score = Math.RandomFloat(0, 0.5) + values[Math.AbsInt(m_aBoard[to])];
			int mover = Math.AbsInt(n[to]);
			if (Math.AbsInt(m_aBoard[from]) == 1 && mover == 5)
				score += 8;

			if (Attacked(n, to, !m_bWhiteToMove))
				score -= values[mover] * 0.9;

			if (Attacked(n, KingSquare(n, !m_bWhiteToMove), m_bWhiteToMove))
				score += 0.3;

			cands.Insert(SquareName(from) + SquareName(to));
			scores.Insert(score);
		}

		BotRanked(cands, scores, moves);
	}

	//------------------------------------------------------------------------------------------------
	protected static string PieceChar(int p)
	{
		array<string> w = {".", "P", "N", "B", "R", "Q", "K"};
		array<string> b = {".", "p", "n", "b", "r", "q", "k"};
		if (p > 0)
			return w[p];

		return b[-p];
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! board (64 pieces, + white), white (seat plays white), last, check (king square), moves (from * 64 + to, own turn only).
	override string Gui(int seat)
	{
		bool white = seat == m_iWhiteSeat;
		string g = "board=" + Ints(m_aBoard) + ";white=" + ROOT_AE3_Util.BoolStr(white) + ";last=" + m_sLastMove;
		int king = KingSquare(m_aBoard, m_bWhiteToMove);
		if (m_iWinner == -1 && Attacked(m_aBoard, king, !m_bWhiteToMove))
			g += ";check=" + king.ToString();

		if (m_iWinner == -1 && seat == m_iTurn)
		{
			array<int> legal = {};
			Legal(m_bWhiteToMove, legal);
			g += ";moves=" + Ints(legal);
		}

		return g;
	}

	//------------------------------------------------------------------------------------------------
	override string View(int seat)
	{
		bool white = seat == m_iWhiteSeat;
		string v;
		if (white)
			v = "You play white (upper case).\n";
		else
			v = "You play black (lower case).\n";

		for (int i = 0; i < 8; i++)
		{
			int rank = 7 - i;
			if (!white)
				rank = i;

			string line = (rank + 1).ToString() + "  ";
			for (int j = 0; j < 8; j++)
			{
				int file = j;
				if (!white)
					file = 7 - j;

				line += PieceChar(m_aBoard[rank * 8 + file]) + " ";
			}

			v += line + "\n";
		}

		if (white)
			v += "   a b c d e f g h";
		else
			v += "   h g f e d c b a";

		if (!m_sLastMove.IsEmpty())
			v += "\nLast move: " + m_sLastMove;

		if (m_iWinner == -1 && Attacked(m_aBoard, KingSquare(m_aBoard, m_bWhiteToMove), !m_bWhiteToMove))
			v += "\nCheck.";

		return v;
	}
}
