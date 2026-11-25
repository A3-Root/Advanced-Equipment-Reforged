//! Finishes a simulated copy after its transfer time.
class ROOT_AE3_CopyJob : ROOT_AE3_Job
{
	ref ROOT_AE3_ShellContext m_Ctx;
	string m_sSrc;
	string m_sDst;
	string m_sShownSrc;
	string m_sShownDst;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!m_Ctx.m_Session || !m_Ctx.m_Session.IsUsable() || !m_Ctx.m_Exec || !m_Ctx.m_Term)
		{
			if (m_Ctx.m_Term)
				m_Ctx.m_Term.m_bBusy = false;

			return;
		}

		if (m_Ctx.IsCancelled())
		{
			m_Ctx.Release();
			return;
		}

		string err;
		if (!m_Ctx.Fs().Copy(m_sSrc, m_sDst, m_Ctx.FsUser(), err))
			m_Ctx.Err(string.Format("cp: cannot copy '%1' to '%2': %3", m_sShownSrc, m_sShownDst, err));

		m_Ctx.Release();
	}
}
