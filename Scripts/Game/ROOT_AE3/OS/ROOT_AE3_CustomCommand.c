//! Command implemented by a script callback (ROOT_AE3_API.AddCustomCommand).
class ROOT_AE3_CustomCommand : ROOT_AE3_Command
{
	string m_sName;
	string m_sPath;
	string m_sDescription;
	string m_sManual;
	ref ROOT_AE3_CommandCallback m_Callback;

	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return m_sName;
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		if (!m_sPath.IsEmpty())
			return m_sPath;

		return "/bin/" + m_sName;
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return m_sDescription;
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		if (!m_sManual.IsEmpty())
			return m_sManual;

		return m_sDescription;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		if (m_Callback)
			m_Callback.Run(ctx, args);
	}
}
