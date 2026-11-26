//! One open game table. Seats belong to computers, so a match can be resumed from the same device.
class ROOT_AE3_GameTable
{
	int m_iId;
	string m_sGame;
	string m_sName;
	ref ROOT_AE3_GameRules m_Rules;
	ref array<ROOT_AE3_ComputerComponent> m_aSeats = {};
	ref array<string> m_aSeatNames = {};
	ref array<int> m_aPrefs = {};
	ref array<bool> m_aBots = {};		//!< per seat: played by the server
	bool m_bBotQueued;
	ref array<ROOT_AE3_ComputerComponent> m_aActiveOwner = {};	//!< computers holding several seats
	ref array<int> m_aActiveSeat = {};							//!< seat each of them plays
	bool m_bStarted;
	int m_iStarter;
	float m_fLastActivity;
	float m_fTurnStart;
	int m_iLastTurn = -1;

	//------------------------------------------------------------------------------------------------
	//! First seated computer.
	ROOT_AE3_ComputerComponent GetHost()
	{
		foreach (ROOT_AE3_ComputerComponent c : m_aSeats)
		{
			if (c)
				return c;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool IsBot(int seat)
	{
		return m_aBots.IsIndexValid(seat) && m_aBots[seat];
	}

	//------------------------------------------------------------------------------------------------
	int SeatOf(ROOT_AE3_ComputerComponent c)
	{
		if (!c)
			return -1;

		return m_aSeats.Find(c);
	}

	//------------------------------------------------------------------------------------------------
	//! Every seat c holds (practice seats included).
	void SeatsOf(ROOT_AE3_ComputerComponent c, notnull array<int> seats)
	{
		if (!c)
			return;

		foreach (int i, ROOT_AE3_ComputerComponent s : m_aSeats)
		{
			if (s == c)
				seats.Insert(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Seat c plays now: the seat to move when c holds it, else the chosen seat, else its first.
	int PlaySeat(ROOT_AE3_ComputerComponent c)
	{
		int first = SeatOf(c);
		if (first < 0)
			return -1;

		if (m_bStarted && m_Rules)
		{
			int turn = m_Rules.GetTurn();
			if (turn >= 0 && m_aSeats.IsIndexValid(turn) && m_aSeats[turn] == c)
				return turn;
		}

		int idx = m_aActiveOwner.Find(c);
		if (idx >= 0 && m_aSeats.IsIndexValid(m_aActiveSeat[idx]) && m_aSeats[m_aActiveSeat[idx]] == c)
			return m_aActiveSeat[idx];

		return first;
	}

	//------------------------------------------------------------------------------------------------
	//! Chooses the seat c plays; false when c does not hold it.
	bool SetPlaySeat(ROOT_AE3_ComputerComponent c, int seat)
	{
		if (!c || !m_aSeats.IsIndexValid(seat) || m_aSeats[seat] != c)
			return false;

		int idx = m_aActiveOwner.Find(c);
		if (idx < 0)
		{
			m_aActiveOwner.Insert(c);
			m_aActiveSeat.Insert(seat);
		}
		else
		{
			m_aActiveSeat[idx] = seat;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Plays the next seat c holds after the current one.
	bool NextPlaySeat(ROOT_AE3_ComputerComponent c)
	{
		array<int> seats = {};
		SeatsOf(c, seats);
		if (seats.Count() < 2)
			return false;

		int pos = seats.Find(PlaySeat(c));
		return SetPlaySeat(c, seats[(pos + 1) % seats.Count()]);
	}

	//------------------------------------------------------------------------------------------------
	string Label()
	{
		string name = m_sName;
		if (name.IsEmpty())
			name = m_Rules.GetTitle();

		string state = "waiting";
		if (m_bStarted)
			state = "playing";

		if (m_Rules.GetWinner() != -1)
			state = "finished";

		return string.Format("#%1 %2 (%3, %4/%5 seats)", m_iId, name, state, m_aSeats.Count(), m_Rules.MaxSeats());
	}
}
