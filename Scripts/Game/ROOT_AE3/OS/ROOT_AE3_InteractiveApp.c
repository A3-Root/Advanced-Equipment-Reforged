//! Full-screen terminal program that takes every typed line until it ends (board games).
class ROOT_AE3_InteractiveApp
{
	//------------------------------------------------------------------------------------------------
	//! Return false when the program ended.
	bool OnLine(ROOT_AE3_Session session, string line)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void OnExit(ROOT_AE3_Session session)
	{
	}
}
