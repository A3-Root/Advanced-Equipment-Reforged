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
	bool m_bStarted;
	int m_iStarter;
	float m_fLastActivity;
	float m_fTurnStart;
	int m_iLastTurn = -1;

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_ComputerComponent GetHost()
	{
		if (m_aSeats.IsEmpty())
			return null;

		return m_aSeats[0];
	}

	//------------------------------------------------------------------------------------------------
	int SeatOf(ROOT_AE3_ComputerComponent c)
	{
		return m_aSeats.Find(c);
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
