//! Reverts the terminal design picked in the menu when the server refuses it.
class ROOT_AE3_DesignReply : ROOT_AE3_GuiReplyHandler
{
	int m_iPrevious;
	ROOT_AE3_TerminalView m_View;

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (ok)
			return;

		ROOT_AE3_Client.Get().m_iDesign = m_iPrevious;
		if (m_View)
			m_View.ApplyDesign();

		if (!payload.IsEmpty())
			SCR_HintManagerComponent.ShowCustomHint(payload[0], "Terminal design", 3);
	}
}
