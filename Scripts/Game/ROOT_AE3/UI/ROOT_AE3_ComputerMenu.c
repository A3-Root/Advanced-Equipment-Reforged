//! Full-screen menu of a computer session: terminal view, desktop view, Snake overlay.
class ROOT_AE3_ComputerMenu : ChimeraMenuBase
{
	protected Widget m_wRoot;
	protected ref ROOT_AE3_TerminalView m_Terminal;
	protected ref ROOT_AE3_DesktopView m_Desktop;
	protected ref ROOT_AE3_SnakeView m_Snake;
	protected bool m_bClosedByServer;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		ROOT_AE3_Client.Get().m_Menu = this;
		ROOT_AE3_Client.Get().m_bMenuOpening = false;
		if (ROOT_AE3_Client.Get().m_DeviceId == RplId.Invalid())
		{
			ROOT_AE3_Client.Get().m_Menu = null;
			Close();
			return;
		}

		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
		Rebuild();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
		StopSnake();
		if (m_Desktop)
			m_Desktop.Destroy();

		if (m_Terminal)
			m_Terminal.Destroy();

		if (!m_bClosedByServer)
			ROOT_AE3_Client.Get().Close();

		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack(float value = 0, EActionTrigger reason = 0)
	{
		if (m_Snake)
		{
			StopSnake();
			return;
		}

		if (m_Desktop && m_Desktop.HandleBack())
			return;

		Close();
	}

	//------------------------------------------------------------------------------------------------
	void CloseFromServer()
	{
		m_bClosedByServer = true;
		Close();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		if (m_Snake)
		{
			m_Snake.Update(tDelta);
			return;
		}

		if (m_Terminal && m_Terminal.IsShown())
			m_Terminal.Update(tDelta);

		if (m_Desktop && m_Desktop.IsShown())
			m_Desktop.Update(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	void Rebuild()
	{
		if (m_Desktop)
			m_Desktop.Destroy();

		if (m_Terminal)
			m_Terminal.Destroy();

		ROOT_AE3_UI.Clear(m_wRoot);
		m_Terminal = new ROOT_AE3_TerminalView(m_wRoot);
		m_Desktop = new ROOT_AE3_DesktopView(m_wRoot);
		ShowMode(ROOT_AE3_Client.Get().m_iMode);
	}

	//------------------------------------------------------------------------------------------------
	void ShowMode(int mode)
	{
		m_Terminal.Show(mode == 0);
		m_Desktop.Show(mode == 1);
	}

	//------------------------------------------------------------------------------------------------
	void OnTerminalChanged()
	{
		if (m_Terminal)
			m_Terminal.Refresh();
	}

	//------------------------------------------------------------------------------------------------
	void OnPromptChanged()
	{
		if (m_Terminal)
			m_Terminal.RefreshPrompt();
	}

	//------------------------------------------------------------------------------------------------
	void SetInputLine(string line)
	{
		if (m_Terminal)
			m_Terminal.SetInput(line);
	}

	//------------------------------------------------------------------------------------------------
	void OnUserChanged()
	{
		if (m_Desktop)
			m_Desktop.OnUserChanged();
	}

	//------------------------------------------------------------------------------------------------
	void Toast(string title, string text)
	{
		if (m_Desktop && m_Desktop.IsShown())
			m_Desktop.Toast(title, text);
		else
			SCR_HintManagerComponent.ShowCustomHint(text, title, 4);
	}

	//------------------------------------------------------------------------------------------------
	void OnGameState(string data)
	{
		if (m_Desktop)
			m_Desktop.OnGameState(data);
	}

	//------------------------------------------------------------------------------------------------
	void StartSnake(bool big)
	{
		StopSnake();
		GetGame().GetWorkspace().SetFocusedWidget(null);
		m_Snake = new ROOT_AE3_SnakeView(m_wRoot, big);
	}

	//------------------------------------------------------------------------------------------------
	void StopSnake()
	{
		if (!m_Snake)
			return;

		m_Snake.Destroy();
		m_Snake = null;
		if (m_Terminal && m_Terminal.IsShown())
			m_Terminal.Focus();
	}
}
