//! Battleship on 10x10 grids. Placement: place <ship 1-5> <square> <h|v>, random, clear, ready.
//! Firing: a square such as b5; a hit fires again. Ships: 5, 4, 3, 3, 2.
class ROOT_AER_GameBattleship : ROOT_AER_GameRules
{
	protected static const int SIZE = 10;
	protected ref array<int> m_aShips = {};		//!< seat * 100 + square: ship number 1-5, 0 water
	protected ref array<int> m_aShots = {};		//!< seat * 100 + square fired at by that seat: 0 none, 1 miss, 2 hit
	protected ref array<bool> m_aReady = {false, false};
	protected ref array<int> m_aLengths = {5, 4, 3, 3, 2};

	//------------------------------------------------------------------------------------------------
	override string GetId()
	{
		return "battleship";
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Battleship";
	}

	//------------------------------------------------------------------------------------------------
	override string GetHelp()
	{
		return "place <ship 1-5> <square> <h|v>, random, clear, ready. Then fire by naming a square such as b5.";
	}

	//------------------------------------------------------------------------------------------------
	override void Start(int seats, int starter)
	{
		super.Start(seats, starter);
		m_aShips.Clear();
		m_aShips.Resize(200);
		m_aShots.Clear();
		m_aShots.Resize(200);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Placing()
	{
		return !m_aReady[0] || !m_aReady[1];
	}

	//------------------------------------------------------------------------------------------------
	override int GetTurn()
	{
		if (m_iWinner != -1 || Placing())
			return -1;

		return m_iTurn;
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearShip(int seat, int ship)
	{
		for (int i = 0; i < 100; i++)
		{
			if (m_aShips[seat * 100 + i] == ship)
				m_aShips[seat * 100 + i] = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool Place(int seat, int ship, int sq, bool vertical, out string status)
	{
		int len = m_aLengths[ship - 1];
		int r = sq / SIZE;
		int f = sq % SIZE;
		if ((vertical && r + len > SIZE) || (!vertical && f + len > SIZE))
		{
			status = "A ship that long does not fit there.";
			return false;
		}

		for (int i = 0; i < len; i++)
		{
			int s = sq + i;
			if (vertical)
				s = sq + i * SIZE;

			int cur = m_aShips[seat * 100 + s];
			if (cur != 0 && cur != ship)
			{
				status = "Another ship is already on those squares.";
				return false;
			}
		}

		ClearShip(seat, ship);
		for (int i = 0; i < len; i++)
		{
			int s = sq + i;
			if (vertical)
				s = sq + i * SIZE;

			m_aShips[seat * 100 + s] = ship;
		}

		status = "Ship placed.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Complete(int seat)
	{
		for (int ship = 1; ship <= 5; ship++)
		{
			bool found;
			for (int i = 0; i < 100; i++)
			{
				if (m_aShips[seat * 100 + i] == ship)
					found = true;
			}

			if (!found)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RandomFleet(int seat)
	{
		for (int i = 0; i < 100; i++)
		{
			m_aShips[seat * 100 + i] = 0;
		}

		for (int ship = 1; ship <= 5; ship++)
		{
			string st;
			for (int tries = 0; tries < 500; tries++)
			{
				if (Place(seat, ship, Math.RandomInt(0, 100), Math.RandomInt(0, 2) == 1, st))
					break;
			}
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

		array<string> w = Words(move);
		if (w.IsEmpty())
		{
			status = "Name a square to fire at, such as b5.";
			return false;
		}

		if (Placing())
		{
			if (m_aReady[seat])
			{
				status = "Your fleet is laid out. Waiting for your opponent.";
				return false;
			}

			switch (w[0])
			{
				case "random":
				{
					RandomFleet(seat);
					status = "Fleet laid out at random.";
					return true;
				}

				case "clear":
				{
					for (int i = 0; i < 100; i++)
					{
						m_aShips[seat * 100 + i] = 0;
					}

					status = "Fleet taken off the grid.";
					return true;
				}

				case "ready":
				{
					if (!Complete(seat))
					{
						status = "Every ship has to be on the grid first.";
						return false;
					}

					m_aReady[seat] = true;
					status = "Fleet ready.";
					if (!Placing())
						status = "Both fleets are ready. Open fire.";

					m_sStatus = status;
					return true;
				}

				case "place":
				{
					if (w.Count() < 4)
					{
						status = GetHelp();
						return false;
					}

					int ship = w[1].ToInt();
					int sq = Square(w[2], SIZE);
					if (ship < 1 || ship > 5)
					{
						status = "No ship of that number.";
						return false;
					}

					if (sq < 0)
					{
						status = "Name a square, such as b5.";
						return false;
					}

					return Place(seat, ship, sq, w[3].StartsWith("v"), status);
				}
			}

			status = "Lay your fleet out first, then say you are ready.";
			return false;
		}

		if (seat != m_iTurn)
		{
			status = "It is not your turn.";
			return false;
		}

		int target = Square(w[0], SIZE);
		if (target < 0)
		{
			status = "Name a square to fire at, such as b5.";
			return false;
		}

		int idx = seat * 100 + target;
		if (m_aShots[idx] != 0)
		{
			status = "You have already fired at that square.";
			return false;
		}

		int enemy = 1 - seat;
		int ship = m_aShips[enemy * 100 + target];
		if (ship == 0)
		{
			m_aShots[idx] = 1;
			status = "Miss.";
			NextTurn();
			m_sStatus = status;
			return true;
		}

		m_aShots[idx] = 2;
		status = "Hit - fire again.";
		bool sunk = true;
		bool all = true;
		for (int i = 0; i < 100; i++)
		{
			int s = m_aShips[enemy * 100 + i];
			if (s != 0 && m_aShots[seat * 100 + i] != 2)
			{
				all = false;
				if (s == ship)
					sunk = false;
			}
		}

		if (sunk)
			status = string.Format("Hit - ship %1 sunk. Fire again.", ship);

		if (all)
		{
			m_iWinner = seat;
			status = "The last ship is sunk - the game is won.";
		}

		m_sStatus = status;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		if (Placing())
		{
			for (int s = 0; s < 2; s++)
			{
				if (!m_aReady[s])
				{
					RandomFleet(s);
					m_aReady[s] = true;
				}
			}

			m_sStatus = "Fleets were laid out at random.";
			return;
		}

		super.OnTimeout();
	}

	//------------------------------------------------------------------------------------------------
	protected string Grid(int seat, bool own)
	{
		string v = "   a b c d e f g h i j\n";
		for (int r = SIZE - 1; r >= 0; r--)
		{
			string line = ROOT_AER_Util.PadLeft((r + 1).ToString(), 2) + " ";
			for (int f = 0; f < SIZE; f++)
			{
				int sq = r * SIZE + f;
				string c = ".";
				if (own)
				{
					int ship = m_aShips[seat * 100 + sq];
					int shot = m_aShots[(1 - seat) * 100 + sq];
					if (ship != 0)
						c = ship.ToString();

					if (shot == 2)
						c = "X";
					else if (shot == 1)
						c = "o";
				}
				else
				{
					int shot = m_aShots[seat * 100 + sq];
					if (shot == 2)
						c = "X";
					else if (shot == 1)
						c = "o";
				}

				line += c + " ";
			}

			v += line + "\n";
		}

		return v;
	}

	//------------------------------------------------------------------------------------------------
	//! Random fleet, ready, then squares next to hits, then a checkerboard pattern, then the rest.
	override void BotMoves(int seat, notnull array<string> moves)
	{
		if (m_iWinner != -1)
			return;

		if (Placing())
		{
			if (m_aReady[seat])
				return;

			if (Complete(seat))
				moves.Insert("ready");
			else
				moves.Insert("random");

			return;
		}

		if (seat != m_iTurn)
			return;

		array<string> nearHit = {};
		array<string> pattern = {};
		array<string> rest = {};
		for (int sq = 0; sq < 100; sq++)
		{
			if (m_aShots[seat * 100 + sq] != 0)
				continue;

			int r = sq / SIZE;
			int f = sq % SIZE;
			bool near = (r > 0 && m_aShots[seat * 100 + sq - SIZE] == 2) || (r < SIZE - 1 && m_aShots[seat * 100 + sq + SIZE] == 2) || (f > 0 && m_aShots[seat * 100 + sq - 1] == 2) || (f < SIZE - 1 && m_aShots[seat * 100 + sq + 1] == 2);
			string name = SquareName(sq, SIZE);
			if (near)
				nearHit.Insert(name);
			else if ((r + f) % 2 == 0)
				pattern.Insert(name);
			else
				rest.Insert(name);
		}

		BotShuffle(nearHit);
		BotShuffle(pattern);
		BotShuffle(rest);
		moves.InsertAll(nearHit);
		moves.InsertAll(pattern);
		moves.InsertAll(rest);
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! ships (own 100 squares), hits (shots received, 1 miss, 2 hit), shots (own shots), sunk (enemy squares of sunk
	//! ships), placing, ready, oppready.
	override string Gui(int seat)
	{
		int enemy = 1 - seat;
		array<int> ships = {};
		array<int> hits = {};
		array<int> shots = {};
		for (int i = 0; i < 100; i++)
		{
			ships.Insert(m_aShips[seat * 100 + i]);
			hits.Insert(m_aShots[enemy * 100 + i]);
			shots.Insert(m_aShots[seat * 100 + i]);
		}

		array<int> sunk = {};
		for (int ship = 1; ship <= 5; ship++)
		{
			array<int> cells = {};
			bool down = true;
			for (int i = 0; i < 100; i++)
			{
				if (m_aShips[enemy * 100 + i] != ship)
					continue;

				cells.Insert(i);
				if (m_aShots[seat * 100 + i] != 2)
					down = false;
			}

			if (down && !cells.IsEmpty())
				sunk.InsertAll(cells);
		}

		string g = "ships=" + Ints(ships) + ";hits=" + Ints(hits) + ";shots=" + Ints(shots) + ";sunk=" + Ints(sunk);
		return g + ";placing=" + ROOT_AER_Util.BoolStr(Placing()) + ";ready=" + ROOT_AER_Util.BoolStr(m_aReady[seat]) + ";oppready=" + ROOT_AER_Util.BoolStr(m_aReady[enemy]);
	}

	//------------------------------------------------------------------------------------------------
	override string View(int seat)
	{
		string v = "Your fleet (1-5 ships, X hit, o miss):\n" + Grid(seat, true);
		v += "Enemy waters:\n" + Grid(seat, false);
		if (Placing())
		{
			if (m_aReady[seat])
				v += "Waiting for your opponent to lay out the fleet.";
			else
				v += "Ship lengths: 1=5 2=4 3=3 4=3 5=2. " + GetHelp();
		}

		return v;
	}
}
