//! Checkers (8x8, dark squares). Men move forward, kings both ways, captures are compulsory and chain
//! with the same piece. Seat 0 plays white (pieces w/W, bottom), seat 1 black (b/B).
class ROOT_AE3_GameCheckers : ROOT_AE3_GameRules
{
	protected ref array<int> m_aBoard = {};	//!< 1 white man, 2 white king, -1 black man, -2 black king
	protected int m_iChain = -1;				//!< square of the piece that must continue capturing
	protected string m_sLastMove;

	//------------------------------------------------------------------------------------------------
	override string GetId()
	{
		return "checkers";
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Checkers";
	}

	//------------------------------------------------------------------------------------------------
	override string GetHelp()
	{
		return "Write a move as two squares, for example c3d4; a jump is c3e5. Captures are compulsory.";
	}

	//------------------------------------------------------------------------------------------------
	override void Start(int seats, int starter)
	{
		super.Start(seats, starter);
		m_aBoard.Clear();
		m_aBoard.Resize(64);
		for (int sq = 0; sq < 64; sq++)
		{
			int r = sq / 8;
			int f = sq % 8;
			if ((r + f) % 2 != 0)
				continue;

			if (r < 3)
				m_aBoard[sq] = 1;
			else if (r > 4)
				m_aBoard[sq] = -1;
		}

		m_iChain = -1;
	}

	//------------------------------------------------------------------------------------------------
	protected int SignOf(int seat)
	{
		if (seat == 0)
			return 1;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Moves of the piece on sq: from * 64 + to; jumps only when jumpsOnly.
	protected void PieceMoves(int sq, bool jumpsOnly, array<int> moves)
	{
		int p = m_aBoard[sq];
		if (p == 0)
			return;

		int sign = 1;
		if (p < 0)
			sign = -1;

		bool king = Math.AbsInt(p) == 2;
		int r = sq / 8;
		int f = sq % 8;
		array<int> dr = {1, 1, -1, -1};
		array<int> df = {1, -1, 1, -1};
		for (int i = 0; i < 4; i++)
		{
			if (!king && dr[i] != sign)
				continue;

			int r1 = r + dr[i];
			int f1 = f + df[i];
			if (r1 < 0 || r1 > 7 || f1 < 0 || f1 > 7)
				continue;

			int mid = r1 * 8 + f1;
			if (m_aBoard[mid] == 0)
			{
				if (!jumpsOnly)
					moves.Insert(sq * 64 + mid);

				continue;
			}

			if (m_aBoard[mid] * sign > 0)
				continue;

			int r2 = r1 + dr[i];
			int f2 = f1 + df[i];
			if (r2 < 0 || r2 > 7 || f2 < 0 || f2 > 7)
				continue;

			int to = r2 * 8 + f2;
			if (m_aBoard[to] == 0)
				moves.Insert(sq * 64 + to);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AllMoves(int sign, bool jumpsOnly, array<int> moves)
	{
		for (int sq = 0; sq < 64; sq++)
		{
			if (m_aBoard[sq] * sign > 0)
				PieceMoves(sq, jumpsOnly, moves);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsJump(int m)
	{
		return Math.AbsInt(m / 64 / 8 - m % 64 / 8) == 2;
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
		move.Replace("-", "");
		move.Replace("x", "");
		move = move.Trim();
		int from = -1;
		int to = -1;
		if (move.Length() >= 4)
		{
			from = Square(move.Substring(0, 2));
			to = Square(move.Substring(2, 2));
		}

		if (from < 0 || to < 0)
		{
			status = "Write a move as two squares, for example c3d4.";
			return false;
		}

		int sign = SignOf(seat);
		if (m_aBoard[from] * sign <= 0)
		{
			status = "That move is not legal.";
			return false;
		}

		if (m_iChain >= 0 && from != m_iChain)
		{
			status = "Carry on capturing with the piece that just jumped.";
			return false;
		}

		array<int> jumps = {};
		AllMoves(sign, true, jumps);
		int m = from * 64 + to;
		array<int> own = {};
		PieceMoves(from, false, own);
		if (!own.Contains(m))
		{
			status = "That move is not legal.";
			return false;
		}

		bool jump = IsJump(m);
		if (!jump && !jumps.IsEmpty())
		{
			status = "A capture is available and has to be taken.";
			return false;
		}

		int p = m_aBoard[from];
		m_aBoard[from] = 0;
		m_aBoard[to] = p;
		if (jump)
			m_aBoard[(from + to) / 2] = 0;

		bool crowned;
		int toRank = to / 8;
		if (Math.AbsInt(p) == 1 && ((p > 0 && toRank == 7) || (p < 0 && toRank == 0)))
		{
			m_aBoard[to] = 2 * sign;
			crowned = true;
		}

		m_sLastMove = SquareName(from) + SquareName(to);
		status = "Move played.";
		if (jump && !crowned)
		{
			array<int> more = {};
			PieceMoves(to, true, more);
			if (!more.IsEmpty())
			{
				m_iChain = to;
				status = "Captured - the same piece can jump again.";
				m_sStatus = status;
				return true;
			}
		}

		m_iChain = -1;
		NextTurn();
		array<int> replies = {};
		AllMoves(-sign, false, replies);
		if (replies.IsEmpty())
		{
			m_iWinner = seat;
			status = "No pieces or no moves left - the game is over.";
		}

		m_sStatus = status;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string View(int seat)
	{
		bool white = seat == 0;
		string v;
		if (white)
			v = "You play white (w, W = king).\n";
		else
			v = "You play black (b, B = king).\n";

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

				int p = m_aBoard[rank * 8 + file];
				string c = ".";
				if ((rank + file) % 2 != 0)
					c = " ";

				switch (p)
				{
					case 1: c = "w"; break;
					case 2: c = "W"; break;
					case -1: c = "b"; break;
					case -2: c = "B"; break;
				}

				line += c + " ";
			}

			v += line + "\n";
		}

		if (white)
			v += "   a b c d e f g h";
		else
			v += "   h g f e d c b a";

		if (!m_sLastMove.IsEmpty())
			v += "\nLast move: " + m_sLastMove;

		if (m_iChain >= 0)
			v += "\nContinue the capture with " + SquareName(m_iChain) + ".";

		return v;
	}
}
