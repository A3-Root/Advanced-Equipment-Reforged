//! Terminal program of a game: hands every line to its front end until quit.
class ROOT_AER_GameTui : ROOT_AER_InteractiveApp
{
	ref ROOT_AER_GameFrontend m_Frontend;

	//------------------------------------------------------------------------------------------------
	override bool OnLine(ROOT_AER_Session session, string line)
	{
		return m_Frontend.OnLine(line);
	}

	//------------------------------------------------------------------------------------------------
	override void OnExit(ROOT_AER_Session session)
	{
		ROOT_AER_GameTableSystem.Get().RemoveFrontend(m_Frontend);
	}
}
