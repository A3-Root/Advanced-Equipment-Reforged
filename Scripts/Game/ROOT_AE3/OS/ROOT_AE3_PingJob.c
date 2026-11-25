//! Prints delayed ping replies.
class ROOT_AE3_PingJob : ROOT_AE3_Job
{
	ref ROOT_AE3_ShellContext m_Ctx;
	string m_sIp;
	int m_iLeft;
	int m_iHops;
	int m_iSent;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!m_Ctx.m_Session || !m_Ctx.m_Session.IsUsable() || !m_Ctx.m_Term || !m_Ctx.m_Term.m_bBusy)
		{
			if (m_Ctx.m_Term)
				m_Ctx.m_Term.m_bBusy = false;

			return;
		}

		m_iSent++;
		int ms = 2 + m_iHops * 3 + Math.RandomInt(0, 4);
		m_Ctx.Out(string.Format("Reply from %1: seq=%2 hops=%3 time=%4 ms", m_sIp, m_iSent, m_iHops, ms));
		m_iLeft--;
		if (m_iLeft > 0)
		{
			Schedule(1000);
			return;
		}

		m_Ctx.Out(string.Format("--- %1 ping statistics: %2 packets transmitted, %2 received, 0%% packet loss", m_sIp, m_iSent));
		m_Ctx.Release();
	}
}
