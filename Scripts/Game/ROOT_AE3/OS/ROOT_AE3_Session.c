//! Server: one player using one computer. Terminal state stays on the computer after the session ends.
class ROOT_AE3_Session : Managed
{
	int m_iPlayerId;
	ROOT_AE3_ComputerComponent m_Computer;
	int m_iMode;
	int m_iCopyJobs;
	protected float m_fLastInput = -10;
	protected string m_sQueuedInput;
	protected float m_fLastMode = -10;	//!< Desktop copies waiting for their transfer time
	protected bool m_bGmRoot;
	protected string m_sSavedUser;
	protected string m_sSavedCwd;
	protected ROOT_AE3_ETermInput m_eSavedInput;
	protected ref array<string> m_aSavedSu = {};
	protected ref array<string> m_aSavedSuCwd = {};
	ROOT_AE3_ComputerComponent m_GuiSshTarget;	//!< Desktop SSH app connection
	string m_sGuiSshUser;
	protected ref array<string> m_aPending = {};
	protected bool m_bFlushQueued;
	protected bool m_bClosed;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_Session(int playerId, ROOT_AE3_ComputerComponent computer)
	{
		m_iPlayerId = playerId;
		m_Computer = computer;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_TerminalState GetTerminal()
	{
		return m_Computer.GetTerminal();
	}

	//------------------------------------------------------------------------------------------------
	bool IsUsable()
	{
		return !m_bClosed && m_Computer && m_Computer.IsRunning();
	}

	//------------------------------------------------------------------------------------------------
	//! mode: 0 terminal, 1 desktop. gmRoot: Game Master access as root (desktop, no access lists, no range).
	//! Returns false with a reason when the player may not use any allowed interface.
	bool Start(int mode, bool gmRoot, out string reason)
	{
		m_bGmRoot = gmRoot;
		int iface = m_Computer.GetInterfaceMode();
		if (!gmRoot)
		{
			if (iface == 0)
				mode = 0;
			else if (iface == 1)
				mode = 1;

			if (!HasAccess(mode))
			{
				if (iface == 2 && HasAccess(1 - mode))
				{
					mode = 1 - mode;
				}
				else
				{
					reason = "Access denied.";
					return false;
				}
			}
		}

		ROOT_AE3_PowerComponent p = m_Computer.GetPower();
		if (p && p.GetState() == ROOT_AE3_EPowerState.STANDBY && !p.TurnOn(reason))
			return false;

		if (gmRoot)
		{
			ROOT_AE3_TerminalState t = GetTerminal();
			m_sSavedUser = t.m_sUser;
			m_sSavedCwd = t.m_sCwd;
			m_eSavedInput = t.m_eInput;
			m_aSavedSu.Copy(t.m_aSuUsers);
			m_aSavedSuCwd.Copy(t.m_aSuCwds);
			t.m_aSuUsers.Clear();
			t.m_aSuCwds.Clear();
			t.m_sUser = "root";
			t.m_sCwd = "/root";
			t.m_eInput = ROOT_AE3_ETermInput.SHELL;
		}

		m_iMode = mode;
		m_Computer.SetOperator(m_iPlayerId);
		SendSession();
		if (m_iMode == 1)
			m_Computer.SetScreen(ROOT_AE3_EScreen.DESKTOP);
		else
			m_Computer.SetScreen(ROOT_AE3_EScreen.TERMINAL);

		if (m_iMode == 1 && GetTerminal().CurrentUser().IsEmpty())
			m_Computer.SetDesktopMirror(m_Computer.GetHostname() + "\nLogin");

		m_Computer.PushMirror(true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Interface allowed for this player (Game Master root sessions: always).
	bool HasAccess(int mode)
	{
		if (m_bGmRoot)
			return true;

		int iface = m_Computer.GetInterfaceMode();
		if (iface != 2 && iface != mode)
			return false;

		return m_Computer.CanAccess(m_iPlayerId, mode);
	}

	//------------------------------------------------------------------------------------------------
	//! Player still alive and next to the computer (Game Master root sessions: always).
	bool PlayerValid(out string reason)
	{
		if (m_bGmRoot)
		{
			if (!m_Computer)
			{
				reason = "Session ended.";
				return false;
			}

			if (ROOT_AE3_GmActions.IsGameMaster(m_iPlayerId))
				return true;

			reason = "Game Master access ended.";
			return false;
		}

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iPlayerId);
		if (!player || !m_Computer)
		{
			reason = "Session ended.";
			return false;
		}

		ChimeraCharacter character = ChimeraCharacter.Cast(player);
		if (character && character.GetCharacterController() && character.GetCharacterController().IsDead())
		{
			reason = "Session ended.";
			return false;
		}

		if (ROOT_AE3_Util.IsCarried(m_Computer.GetOwner()))
		{
			reason = "The computer was picked up.";
			return false;
		}

		if (vector.Distance(player.GetOrigin(), m_Computer.GetOwner().GetOrigin()) > ROOT_AE3_Manager.USE_RANGE + 1)
		{
			reason = "You moved away from the computer.";
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string DeviceIdString()
	{
		ROOT_AE3_DeviceComponent dev = ROOT_AE3_DeviceComponent.Of(m_Computer.GetOwner());
		int id = -1;
		if (dev)
			id = dev.GetRplId();

		return id.ToString();
	}

	//------------------------------------------------------------------------------------------------
	protected void FlushInput()
	{
		if (m_bClosed || m_sQueuedInput.IsEmpty())
			return;

		string key = m_sQueuedInput;
		m_sQueuedInput = "";
		m_fLastInput = -10;
		OnKey(key);
	}

	//------------------------------------------------------------------------------------------------
	void SendSession()
	{
		ROOT_AE3_TerminalState t = GetTerminal();
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		string user = t.CurrentUser();
		array<string> f = {
			m_Computer.GetHostname(),
			m_iMode.ToString(),
			s.PackClient(),
			m_Computer.GetDesign().ToString(),
			user,
			m_Computer.GetInterfaceMode().ToString(),
			ROOT_AE3_Util.Last(t.Tail(ROOT_AE3_TerminalState.MAX_LINES), 32768),
			ROOT_AE3_Shell.PromptOf(m_Computer, t),
			ROOT_AE3_Util.BoolStr(t.m_eInput == ROOT_AE3_ETermInput.PASSWORD || t.m_bPromptHidden),
			ROOT_AE3_Codec.JoinSub(m_Computer.GetHistory(user)),
			m_Computer.GetPref(user, "theme", s.m_iDesktopDefaultTheme.ToString()),
			m_Computer.GetPref(user, "wallpaper", m_Computer.GetPref("", "wallpaper", "1")),
			DeviceIdString()
		};
		ROOT_AE3_Net.SendToPlayer(m_iPlayerId, ROOT_AE3_EMsg.SESSION, ROOT_AE3_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	void Close()
	{
		GetGame().GetCallqueue().Remove(FlushInput);
		if (m_bClosed)
			return;

		m_bClosed = true;
		if (m_Computer)
		{
			ROOT_AE3_TerminalState t = m_Computer.GetTerminal();
			t.m_bBusy = false;
			if (m_bGmRoot)
			{
				t.m_sUser = m_sSavedUser;
				t.m_sCwd = m_sSavedCwd;
				t.m_eInput = m_eSavedInput;
				t.m_aSuUsers.Copy(m_aSavedSu);
				t.m_aSuCwds.Copy(m_aSavedSuCwd);
				t.m_SshTarget = null;
				t.m_sSshUser = "";
				t.m_sSshCwd = "/";
				t.m_aSshSu.Clear();
				t.m_aSshSuCwd.Clear();
				t.m_Prompt = null;
				t.m_App = null;
				t.m_iGeneration++;
			}
			else
			{
				if (!t.m_aSuCwds.IsEmpty())
					t.m_sCwd = t.m_aSuCwds[0];

				t.m_aSuUsers.Clear();
				t.m_aSuCwds.Clear();
				t.m_SshTarget = null;
				t.m_sSshUser = "";
				t.m_sSshCwd = "/";
				t.m_aSshSu.Clear();
				t.m_aSshSuCwd.Clear();
				t.m_Prompt = null;
				t.m_App = null;
				t.m_iGeneration++;
				if (t.m_eInput == ROOT_AE3_ETermInput.PROMPT)
					t.m_eInput = ROOT_AE3_ETermInput.SHELL;

				if (t.m_eInput == ROOT_AE3_ETermInput.PASSWORD)
					t.m_eInput = ROOT_AE3_ETermInput.LOGIN;

				t.m_sLoginName = "";
				if (ROOT_AE3_SettingsManager.Get().m_bLogoutOnSessionEnd && !t.m_sUser.IsEmpty())
					ROOT_AE3_Shell.Logout(this);
			}

			ROOT_AE3_GameTableSystem.Get().OnSessionClosed(this);
			if (m_Computer.GetScreen() == ROOT_AE3_EScreen.DESKTOP)
				m_Computer.SetScreen(ROOT_AE3_EScreen.TERMINAL);

			m_Computer.SetOperator(0);
			m_Computer.SetScreenInput("");
			m_Computer.PushMirror(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnLine(string line)
	{
		string clean = ROOT_AE3_Util.Ascii(ROOT_AE3_Codec.Clean(line));
		clean.Replace("\n", " ");
		clean.Replace("\t", " ");
		ROOT_AE3_Shell.OnLine(this, clean);
		m_Computer.SetScreenInput("");
	}

	//------------------------------------------------------------------------------------------------
	void OnTab(string line)
	{
		string completed = ROOT_AE3_Shell.Complete(this, line);
		ROOT_AE3_Net.SendToPlayer(m_iPlayerId, ROOT_AE3_EMsg.TERM_SET_LINE, completed);
	}

	//------------------------------------------------------------------------------------------------
	//! "INPUT:<text>" mirrors the typed line, "CTRL_C" cancels the prompt or program, "MODE:<n>" switches view.
	void OnKey(string key)
	{
		if (key.StartsWith("INPUT:"))
		{
			ROOT_AE3_TerminalState t = GetTerminal();
			if (t.m_eInput == ROOT_AE3_ETermInput.PASSWORD || t.m_bPromptHidden)
				return;

			float nowIn = GetGame().GetWorld().GetWorldTime() / 1000.0;
			float interval = Math.Max(0.1, ROOT_AE3_SettingsManager.Get().m_fUiKeystrokeSyncInterval);
			if (nowIn - m_fLastInput < interval)
			{
				m_sQueuedInput = key;
				GetGame().GetCallqueue().Remove(FlushInput);
				GetGame().GetCallqueue().CallLater(FlushInput, interval * 1000, false);
				return;
			}

			m_fLastInput = nowIn;
			string input = key.Substring(6, key.Length() - 6);
			if (input.Length() > 256)
				input = input.Substring(0, 256);

			if (m_iMode == 0)
				m_Computer.SetScreenInput(ROOT_AE3_Shell.MaskSecrets(input));

			return;
		}

		if (key.StartsWith("MODE:"))
		{
			float nowMode = GetGame().GetWorld().GetWorldTime() / 1000.0;
			if (nowMode - m_fLastMode < 0.5)
				return;

			m_fLastMode = nowMode;
			SwitchMode(Math.ClampInt(key.Substring(5, key.Length() - 5).ToInt(), 0, 1));
			return;
		}

		if (key == "CTRL_C" && m_iMode == 0 && HasAccess(0))
			ROOT_AE3_Shell.Interrupt(this);
	}

	//------------------------------------------------------------------------------------------------
	//! 0 terminal, 1 desktop; respects the interface mode.
	void SwitchMode(int mode)
	{
		int iface = m_Computer.GetInterfaceMode();
		if (!m_bGmRoot && ((mode == 1 && iface == 0) || (mode == 0 && iface == 1) || !HasAccess(mode)))
		{
			Out("The interface is not available on this computer.");
			Flush();
			return;
		}

		m_iMode = mode;
		if (mode == 1)
			m_Computer.SetScreen(ROOT_AE3_EScreen.DESKTOP);
		else
			m_Computer.SetScreen(ROOT_AE3_EScreen.TERMINAL);

		m_Computer.PushMirror(true);
		ROOT_AE3_Net.SendToPlayer(m_iPlayerId, ROOT_AE3_EMsg.MODE, mode.ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! Appends output lines to the terminal and queues them for the client.
	void Out(string text)
	{
		array<string> lines = {};
		text.Split("\n", lines, false);
		ROOT_AE3_TerminalState t = GetTerminal();
		foreach (string l : lines)
		{
			if (l.Length() > ROOT_AE3_TerminalState.MAX_LINE_CHARS)
				l = l.Substring(0, ROOT_AE3_TerminalState.MAX_LINE_CHARS);

			t.AddLine(l);
			m_aPending.Insert(l);
		}

		int excess = m_aPending.Count() - ROOT_AE3_TerminalState.MAX_LINES;
		if (excess > 0)
		{
			array<string> keep = {};
			for (int i = excess; i < m_aPending.Count(); i++)
			{
				keep.Insert(m_aPending[i]);
			}

			m_aPending = keep;
		}

		if (!m_bFlushQueued)
		{
			m_bFlushQueued = true;
			GetGame().GetCallqueue().CallLater(Flush, 0, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the terminal tail again after output was dropped.
	void Resync()
	{
		if (m_bClosed)
			return;

		ROOT_AE3_Net.SendToPlayer(m_iPlayerId, ROOT_AE3_EMsg.TERM_CLEAR, "");
		ROOT_AE3_Net.SendToPlayer(m_iPlayerId, ROOT_AE3_EMsg.TERM_OUT, ROOT_AE3_Util.Last(GetTerminal().Tail(100), 16384));
		SendPrompt();
	}

	//------------------------------------------------------------------------------------------------
	void Flush()
	{
		m_bFlushQueued = false;
		if (m_bClosed)
			return;

		if (!m_aPending.IsEmpty())
		{
			string text;
			foreach (int i, string l : m_aPending)
			{
				if (i > 0)
					text += "\n";

				text += l;
			}

			m_aPending.Clear();
			if (text.Length() > 65536)
				text = text.Substring(text.Length() - 65536, 65536);

			ROOT_AE3_Net.SendToPlayer(m_iPlayerId, ROOT_AE3_EMsg.TERM_OUT, text);
		}

		if (m_Computer)
			m_Computer.PushMirror();
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		GetTerminal().m_aBuffer.Clear();
		m_aPending.Clear();
		ROOT_AE3_Net.SendToPlayer(m_iPlayerId, ROOT_AE3_EMsg.TERM_CLEAR, "");
		m_Computer.PushMirror(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the current prompt (after flushing output).
	void SendPrompt()
	{
		Flush();
		ROOT_AE3_TerminalState t = GetTerminal();
		bool hidden = t.m_eInput == ROOT_AE3_ETermInput.PASSWORD || (t.m_eInput == ROOT_AE3_ETermInput.PROMPT && t.m_bPromptHidden);
		array<string> f = {ROOT_AE3_Shell.PromptOf(m_Computer, t), ROOT_AE3_Util.BoolStr(hidden), ROOT_AE3_Util.BoolStr(t.m_bBusy)};
		ROOT_AE3_Net.SendToPlayer(m_iPlayerId, ROOT_AE3_EMsg.TERM_PROMPT, ROOT_AE3_Codec.Join(f));
		m_Computer.PushMirror();
	}

	//------------------------------------------------------------------------------------------------
	void Notify(string title, string text)
	{
		array<string> f = {title, text};
		ROOT_AE3_Net.SendToPlayer(m_iPlayerId, ROOT_AE3_EMsg.NOTIFY, ROOT_AE3_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	//! Password result with a per-player delay after failures (1 s per failure, up to 5 s).
	bool CheckAuth(bool ok)
	{
		return ROOT_AE3_Manager.Get().CheckAuth(m_iPlayerId, ok);
	}

	//------------------------------------------------------------------------------------------------
	bool IsGmRoot()
	{
		return m_bGmRoot;
	}

	//------------------------------------------------------------------------------------------------
	//! Logged-in user of the computer (terminal and desktop share it).
	string GetUser()
	{
		return GetTerminal().CurrentUser();
	}
}
