//! Terminal program of a game: hands every line to its front end until quit.
class ROOT_AE3_GameTui : ROOT_AE3_InteractiveApp
{
	ref ROOT_AE3_GameFrontend m_Frontend;

	//------------------------------------------------------------------------------------------------
	override bool OnLine(ROOT_AE3_Session session, string line)
	{
		return m_Frontend.OnLine(line);
	}

	//------------------------------------------------------------------------------------------------
	override void OnExit(ROOT_AE3_Session session)
	{
		ROOT_AE3_GameTableSystem.Get().RemoveFrontend(m_Frontend);
	}
}
