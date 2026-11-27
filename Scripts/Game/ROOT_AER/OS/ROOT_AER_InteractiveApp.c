//! Full-screen terminal program that takes every typed line until it ends (board games).
class ROOT_AER_InteractiveApp
{
	//------------------------------------------------------------------------------------------------
	//! Return false when the program ended.
	bool OnLine(ROOT_AER_Session session, string line)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void OnExit(ROOT_AER_Session session)
	{
	}
}
