//! SSH pull / push finished after the network transfer time.
class ROOT_AE3_SshTransferJob : ROOT_AE3_Job
{
	ROOT_AE3_Session m_Session;
	string m_sRid;
	string m_sCmd;
	ref ROOT_AE3_FsNode m_Node;
	ROOT_AE3_FsNode m_Target;
	string m_sTargetPath;				//!< Target folder path in m_TargetFs
	ROOT_AE3_Filesystem m_TargetFs;
	ROOT_AE3_Filesystem m_Fs;	//!< Filesystem holding the target (space check)
	ROOT_AE3_ComputerComponent m_Remote;
	string m_sWriteUser;	//!< Filesystem user that writes the target
	string m_sOwner;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!m_Session || !m_Session.IsUsable() || !m_Target)
			return;

		if (!m_Node)
		{
			ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, false, m_sCmd, ROOT_AE3_Filesystem.ERR_PERMISSION);
			return;
		}

		if (m_TargetFs)
		{
			string terr;
			ROOT_AE3_FsNode now = m_TargetFs.Resolve(m_sTargetPath, m_sWriteUser, terr);
			if (now != m_Target)
			{
				ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, false, m_sCmd, "The target folder changed.");
				return;
			}
		}

		if (m_Session.m_GuiSshTarget != m_Remote || !m_Target.HasPermission(m_sWriteUser, 1))
		{
			ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, false, m_sCmd, "Connection closed or permission denied.");
			return;
		}

		int hops;
		if (!m_Remote || !m_Remote.IsRunning() || !ROOT_AE3_NetworkSystem.Get().CanReach(m_Session.m_Computer.GetNetwork(), m_Remote.GetNetwork(), hops))
		{
			ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, false, m_sCmd, "Connection lost.");
			return;
		}

		if (m_Target.GetChild(m_Node.m_sName))
		{
			ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, false, m_sCmd, ROOT_AE3_Filesystem.ERR_EXISTS + ": " + m_Node.m_sName);
			return;
		}

		string err;
		if (m_Fs && !m_Fs.Fits(m_Node, err))
		{
			ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, false, m_sCmd, err);
			return;
		}

		m_Node.SetOwnerRecursive(m_sOwner);
		m_Target.SetChild(m_Node);
		if (m_TargetFs)
			m_TargetFs.NotifyChanged(m_sTargetPath + "/" + m_Node.m_sName);
		ROOT_AE3_GuiRouter.Msg(m_Session, m_sRid, true, m_sCmd, "Transferred " + m_Node.m_sName);
	}
}
