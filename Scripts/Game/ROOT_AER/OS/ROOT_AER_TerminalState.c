enum ROOT_AER_ETermInput
{
	SHELL,
	LOGIN,
	PASSWORD,
	PROMPT
}

//! Terminal of one computer. Kept on the computer between sessions (standby, closing the menu).
class ROOT_AER_TerminalState : Managed
{
	static const int MAX_LINES = 400;

	ref array<string> m_aBuffer = {};
	static const int MAX_LINE_CHARS = 1024;	//!< Characters kept per terminal line
	ROOT_AER_ETermInput m_eInput = ROOT_AER_ETermInput.LOGIN;
	string m_sUser;					//!< Logged-in user; empty = login screen.
	string m_sLoginName;			//!< User name typed at LOGIN, waiting for PASSWORD.
	string m_sCwd = "/";
	ref array<string> m_aSuUsers = {};
	ref array<string> m_aSuCwds = {};
	ROOT_AER_ComputerComponent m_SshTarget;
	string m_sSshUser;
	string m_sSshCwd = "/";
	ref array<string> m_aSshSu = {};		//!< Previous SSH accounts (su inside SSH)
	ref array<string> m_aSshSuCwd = {};
	ref ROOT_AER_PromptHandler m_Prompt;
	string m_sPromptText;
	bool m_bPromptHidden;
	bool m_bBusy;
	int m_iGeneration;	//!< Bumped by Ctrl+C and login changes; pending jobs of older generations are dropped
	ref ROOT_AER_InteractiveApp m_App;	//!< Running full-screen terminal program (games).

	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		m_aBuffer.Clear();
		m_eInput = ROOT_AER_ETermInput.LOGIN;
		m_sUser = "";
		m_sLoginName = "";
		m_sCwd = "/";
		m_aSuUsers.Clear();
		m_aSuCwds.Clear();
		m_SshTarget = null;
		m_sSshUser = "";
		m_sSshCwd = "/";
		m_aSshSu.Clear();
		m_aSshSuCwd.Clear();
		m_Prompt = null;
		m_bBusy = false;
		m_App = null;
	}

	//------------------------------------------------------------------------------------------------
	void AddLine(string line)
	{
		if (line.Length() > MAX_LINE_CHARS)
			line = line.Substring(0, MAX_LINE_CHARS);

		m_aBuffer.Insert(line);
		if (m_aBuffer.Count() > MAX_LINES)
			m_aBuffer.RemoveOrdered(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Effective user of the shell (su target or login user).
	string CurrentUser()
	{
		if (!m_aSuUsers.IsEmpty())
			return m_aSuUsers[m_aSuUsers.Count() - 1];

		return m_sUser;
	}

	//------------------------------------------------------------------------------------------------
	bool InSsh()
	{
		return m_SshTarget != null;
	}

	//------------------------------------------------------------------------------------------------
	string GetCwd()
	{
		if (InSsh())
			return m_sSshCwd;

		return m_sCwd;
	}

	//------------------------------------------------------------------------------------------------
	void SetCwd(string cwd)
	{
		if (InSsh())
			m_sSshCwd = cwd;
		else
			m_sCwd = cwd;
	}

	//------------------------------------------------------------------------------------------------
	//! Text of the last lines.
	string Tail(int lines)
	{
		string r;
		int start = Math.Max(0, m_aBuffer.Count() - lines);
		for (int i = start; i < m_aBuffer.Count(); i++)
		{
			if (i > start)
				r += "\n";

			r += m_aBuffer[i];
		}

		return r;
	}
}
