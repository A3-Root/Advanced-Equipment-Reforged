//! Desktop copy finished after the simulated transfer time.
class ROOT_AE3_GuiCopyJob : ROOT_AE3_Job
{
	ROOT_AE3_Session m_Session;
	string m_sRid;
	string m_sSrc;
	string m_sDst;
	string m_sUser;
	ROOT_AE3_Filesystem m_DstFs;
	string m_sLoginUser;	//!< Session user when the copy was requested

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!m_Session || !m_Session.IsUsable())
			return;

		m_Session.m_iCopyJobs = Math.Max(0, m_Session.m_iCopyJobs - 1);
		if (m_Session.GetTerminal().CurrentUser() != m_sLoginUser)
		{
			ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, false, "fs_copy", "Copy cancelled: user changed.");
			return;
		}

		string err;
		ROOT_AE3_Filesystem fs = m_Session.m_Computer.GetFilesystem();
		if (!fs.Copy(m_sSrc, m_sDst, m_Session.m_Computer.FsUser(m_sLoginUser), err, m_DstFs))
		{
			ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, false, "fs_copy", err);
			return;
		}

		ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, true, "fs_copy", "Copied to " + m_sDst);
	}
}
