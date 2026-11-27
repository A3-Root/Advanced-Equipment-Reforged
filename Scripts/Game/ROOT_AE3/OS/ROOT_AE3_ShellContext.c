//! Execution context of one command line: executing computer (local or SSH target), user, output target.
class ROOT_AE3_ShellContext
{
	ROOT_AE3_Session m_Session;
	ROOT_AE3_TerminalState m_Term;
	ROOT_AE3_ComputerComponent m_Exec;
	string m_sUser;
	bool m_bSudo;
	string m_sCommand;
	ref array<string> m_aCapture;
	string m_sRedirectTarget;
	bool m_bRedirectAppend;
	bool m_bError;
	protected int m_iHolds;
	protected int m_iOutput;
	static const int MAX_OUTPUT = 65536;	//!< Characters of output per command line
	protected int m_iGeneration;
	protected bool m_bFinished;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_ShellContext(ROOT_AE3_Session session, ROOT_AE3_ComputerComponent exec, string user)
	{
		m_Session = session;
		m_Term = session.GetTerminal();
		m_Exec = exec;
		m_sUser = user;
		m_iGeneration = m_Term.m_iGeneration;
	}

	//------------------------------------------------------------------------------------------------
	//! Interrupted (Ctrl+C) or user changed since this command started.
	bool IsCancelled()
	{
		return !m_Term || m_Term.m_iGeneration != m_iGeneration;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_Filesystem Fs()
	{
		return m_Exec.GetFilesystem();
	}

	//------------------------------------------------------------------------------------------------
	//! Filesystem user: root under sudo, root for elevated sudoers, else the shell user.
	string FsUser()
	{
		if (m_bSudo)
			return "root";

		return m_Exec.FsUser(m_sUser);
	}

	//------------------------------------------------------------------------------------------------
	string Cwd()
	{
		return m_Term.GetCwd();
	}

	//------------------------------------------------------------------------------------------------
	string Abs(string path)
	{
		return ROOT_AE3_Filesystem.Normalize(Cwd(), path, m_sUser);
	}

	//------------------------------------------------------------------------------------------------
	//! Standard output: captured for redirection or printed.
	void Out(string text)
	{
		if (!Budget(text))
			return;

		if (m_aCapture)
		{
			m_aCapture.Insert(text);
			return;
		}

		m_Session.Out(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Output of one command line stops after MAX_OUTPUT characters.
	protected bool Budget(string text)
	{
		if (m_iOutput > MAX_OUTPUT)
			return false;

		m_iOutput += text.Length() + 1;
		if (m_iOutput <= MAX_OUTPUT)
			return true;

		m_Session.Out("output truncated");
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Error output: always printed; plays the error sound.
	void Err(string text)
	{
		if (!Budget(text))
			return;

		m_bError = true;
		m_Session.Out(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps the prompt hidden until Release (asynchronous commands).
	void Hold()
	{
		m_iHolds++;
		m_Term.m_bBusy = true;
	}

	//------------------------------------------------------------------------------------------------
	void Release()
	{
		m_iHolds--;
		if (m_iHolds <= 0)
			Finish();
	}

	//------------------------------------------------------------------------------------------------
	bool IsHeld()
	{
		return m_iHolds > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Writes redirected output, plays the error sound and shows the prompt.
	void Finish()
	{
		if (m_bFinished)
			return;

		m_bFinished = true;
		if (IsCancelled())
			return;

		m_Term.m_bBusy = false;
		if (m_aCapture)
		{
			string data;
			foreach (int i, string l : m_aCapture)
			{
				if (i > 0)
					data += "\n";

				data += l;
			}

			if (!data.IsEmpty())
				data += "\n";

			string err;
			string target = Abs(m_sRedirectTarget);
			if (!Fs().WriteFile(target, data, FsUser(), m_bRedirectAppend, err))
				Err(m_sRedirectTarget + ": " + err);

			m_aCapture = null;
		}

		if (m_bError && ROOT_AE3_SettingsManager.Get().m_bEnableErrorSound)
			ROOT_AE3_Sound.Play(m_Session.m_Computer.GetOwner(), ROOT_AE3_Sound.ERROR);

		m_Session.SendPrompt();
	}

	//------------------------------------------------------------------------------------------------
	//! Appends a line to a log file of the executing computer.
	void Log(string file, string message)
	{
		ROOT_AE3_Shell.WriteLog(m_Exec, file, message);
	}
}
