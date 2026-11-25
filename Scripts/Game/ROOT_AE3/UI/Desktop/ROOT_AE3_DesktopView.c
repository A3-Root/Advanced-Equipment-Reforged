//! armaOS desktop: login screen, wallpaper, icons, windows, taskbar, start menu, notifications.
class ROOT_AE3_DesktopView : ROOT_AE3_GuiReplyHandler
{
	protected static const int ID_BG = 1;
	protected static const int ID_START = 2;
	protected static const int ID_LOGIN = 3;
	protected static const int ID_USER = 4;
	protected static const int ID_PASS = 5;
	protected static const int ID_SIGNOUT = 6;
	protected static const int ID_SHUTDOWN = 7;
	protected static const int ID_STANDBY = 8;
	protected static const int ID_TERMINAL = 9;
	protected static const int ID_TOAST = 10;
	protected static const int ID_ICON = 100;
	protected static const int ID_MENU = 300;
	protected static const int ID_TASK = 500;
	protected static const float BAR_H = 40;

	protected FrameWidget m_wRoot;
	protected FrameWidget m_wWindows;
	protected FrameWidget m_wTaskbar;
	protected FrameWidget m_wMenu;
	protected FrameWidget m_wToast;
	protected TextWidget m_wClock;
	protected TextWidget m_wTray;
	protected EditBoxWidget m_wUser;
	protected EditBoxWidget m_wPass;
	protected TextWidget m_wLoginMsg;
	protected ref array<ref ROOT_AE3_Window> m_aWindows = {};
	protected ref array<typename> m_aApps = {};
	protected ROOT_AE3_Window m_Focus;
	protected int m_iZ = 10;
	protected float m_fW, m_fH;
	protected bool m_bShown;
	protected float m_fClockTimer;
	protected float m_fToastTimer;
	protected string m_sTray;
	protected string m_sBuiltKey = "-";
	protected ref ROOT_AE3_ForwardListener m_TaskbarListener;
	protected ref ROOT_AE3_ForwardListener m_MenuListener;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_DesktopView(Widget parent)
	{
		float sw, sh;
		ROOT_AE3_UI.ScreenSize(sw, sh);
		array<float> fractions = {1, 0.85, 0.65, 0.45};
		float frac = fractions[Math.ClampInt(ROOT_AE3_SettingsManager.Get().m_iDesktopSize, 0, 3)];
		m_fW = sw * frac;
		m_fH = sh * frac;
		m_wRoot = ROOT_AE3_UI.Frame(parent, (sw - m_fW) / 2, (sh - m_fH) / 2, m_fW, m_fH);
		ROOT_AE3_Theme.s_iTheme = ROOT_AE3_Client.Get().m_sTheme.ToInt();
		ROOT_AE3_AppRegistry.GetApps(m_aApps);
		Build();
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		foreach (ROOT_AE3_Window w : m_aWindows)
		{
			w.Destroy();
		}

		m_aWindows.Clear();
		ReleaseHandlers();
		ROOT_AE3_Client.Get().Forget(this);
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	bool IsShown()
	{
		return m_bShown;
	}

	//------------------------------------------------------------------------------------------------
	void Show(bool show)
	{
		m_bShown = show;
		m_wRoot.SetVisible(show);
		if (show && BuildKey() != m_sBuiltKey)
			Build();
	}

	//------------------------------------------------------------------------------------------------
	//! What the desktop depends on: user, theme and wallpaper.
	protected string BuildKey()
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		return c.m_sUser + "|" + c.m_sTheme + "|" + c.m_sWallpaper;
	}

	//------------------------------------------------------------------------------------------------
	FrameWidget GetWindowLayer()
	{
		return m_wWindows;
	}

	//------------------------------------------------------------------------------------------------
	void GetArea(out float w, out float h)
	{
		w = m_fW;
		h = m_fH - BAR_H;
	}

	//------------------------------------------------------------------------------------------------
	int NextZ()
	{
		m_iZ++;
		return m_iZ;
	}

	//------------------------------------------------------------------------------------------------
	void SetFocusWindow(ROOT_AE3_Window w)
	{
		m_Focus = w;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoggedIn()
	{
		return !ROOT_AE3_Client.Get().m_sUser.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Full rebuild (theme, user or wallpaper changed). Open windows are closed.
	void Build()
	{
		m_sBuiltKey = BuildKey();
		foreach (ROOT_AE3_Window w : m_aWindows)
		{
			w.Destroy();
		}

		m_aWindows.Clear();
		ReleaseHandlers();
		ROOT_AE3_UI.Clear(m_wRoot);
		ROOT_AE3_Theme.s_iTheme = ROOT_AE3_Client.Get().m_sTheme.ToInt();
		ROOT_AE3_UI.Rect(m_wRoot, 0, 0, m_fW, m_fH, ROOT_AE3_Theme.Desktop(), ID_BG, this);
		ResourceName wall = ROOT_AE3_AppRegistry.Wallpaper(ROOT_AE3_Client.Get().m_sWallpaper.ToInt());
		if (!wall.IsEmpty())
			ROOT_AE3_UI.Picture(m_wRoot, 0, 0, m_fW, m_fH - BAR_H, wall);

		if (!LoggedIn())
		{
			BuildLogin();
			return;
		}

		BuildIcons();
		m_wWindows = ROOT_AE3_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH - BAR_H, false);
		BuildTaskbar();
		m_wMenu = null;
		m_wToast = null;
		PushMirror();
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildLogin()
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		float w = 380;
		float h = 250;
		float x = (m_fW - w) * 0.5;
		float y = (m_fH - h) * 0.5;
		ROOT_AE3_UI.Rect(m_wRoot, x, y, w, h, ROOT_AE3_UI.Rgb(0, 0, 0, 0.75));
		ROOT_AE3_UI.Text(m_wRoot, x + 24, y + 18, w - 48, 30, "armaOS  -  " + c.m_sHostname, 22);
		ROOT_AE3_UI.Text(m_wRoot, x + 24, y + 64, 90, 22, "User", 16);
		ROOT_AE3_UI.Rect(m_wRoot, x + 120, y + 60, w - 144, 28, ROOT_AE3_UI.Rgb(0.08, 0.08, 0.09));
		m_wUser = ROOT_AE3_UI.Edit(m_wRoot, x + 124, y + 62, w - 152, 24, "", ID_USER, this);
		ROOT_AE3_UI.Text(m_wRoot, x + 24, y + 104, 90, 22, "Password", 16);
		ROOT_AE3_UI.Rect(m_wRoot, x + 120, y + 100, w - 144, 28, ROOT_AE3_UI.Rgb(0.08, 0.08, 0.09));
		m_wPass = ROOT_AE3_UI.Edit(m_wRoot, x + 124, y + 102, w - 152, 24, "", ID_PASS, this, true);
		ROOT_AE3_UI.Button(m_wRoot, x + 120, y + 144, 120, 30, "Log in", ID_LOGIN, this, ROOT_AE3_Theme.Accent());
		if (c.m_iInterface != 1)
			ROOT_AE3_UI.Button(m_wRoot, x + 250, y + 144, 106, 30, "Terminal", ID_TERMINAL, this);

		m_wLoginMsg = ROOT_AE3_UI.Text(m_wRoot, x + 24, y + 190, w - 48, 40, "", 14, ROOT_AE3_UI.Rgb(1, 0.6, 0.5), false, true);
		if (m_wUser)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_wUser);
			m_wUser.ActivateWriteMode();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildIcons()
	{
		int col, row;
		int perCol = Math.Max(1, Math.Floor((m_fH - BAR_H - 20) / 96));
		bool files = ROOT_AE3_SettingsManager.Get().m_bDesktopEnableFileBrowsing;
		foreach (int i, typename t : m_aApps)
		{
			ROOT_AE3_App app = ROOT_AE3_App.Cast(t.Spawn());
			if (!app || !app.IsListed() || !app.IsOnDesktop() || (app.NeedsFileBrowsing() && !files))
				continue;

			float x = 16 + col * 92;
			float y = 16 + row * 96;
			ROOT_AE3_UI.Rect(m_wRoot, x, y, 84, 88, ROOT_AE3_UI.Rgb(1, 1, 1, 0.04), ID_ICON + i, this);
			ROOT_AE3_UI.Icon(m_wRoot, x + 22, y + 6, 40, app.GetIcon());
			TextWidget t2 = ROOT_AE3_UI.Text(m_wRoot, x + 2, y + 50, 80, 36, app.GetTitle(), 13, null, false, true);
			t2.SetShadow(1, 0xFF000000, 1, 1, 1);
			row++;
			if (row >= perCol)
			{
				row = 0;
				col++;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildTaskbar()
	{
		m_wTaskbar = ROOT_AE3_UI.Frame(m_wRoot, 0, m_fH - BAR_H, m_fW, BAR_H);
		m_wTaskbar.SetZOrder(100000);
		RefreshTaskbar();
	}

	//------------------------------------------------------------------------------------------------
	void RefreshTaskbar()
	{
		if (!m_wTaskbar)
			return;

		ROOT_AE3_UI.Clear(m_wTaskbar);
		m_TaskbarListener = new ROOT_AE3_ForwardListener(this);
		ROOT_AE3_UI.Rect(m_wTaskbar, 0, 0, m_fW, BAR_H, ROOT_AE3_Theme.Bar());
		ROOT_AE3_UI.Button(m_wTaskbar, 4, 4, 96, BAR_H - 8, "armaOS", ID_START, m_TaskbarListener, ROOT_AE3_Theme.Accent(), ROOT_AE3_UI.Rgb(1, 1, 1));
		float x = 108;
		foreach (int i, ROOT_AE3_Window w : m_aWindows)
		{
			Color bg = ROOT_AE3_Theme.Button();
			if (w.IsMinimized())
				bg = ROOT_AE3_UI.Rgb(0.2, 0.2, 0.22, 0.6);

			ROOT_AE3_UI.Button(m_wTaskbar, x, 4, 150, BAR_H - 8, w.m_App.GetTitle(), ID_TASK + i, m_TaskbarListener, bg, ROOT_AE3_Theme.Text(), 14);
			x += 156;
			if (x > m_fW - 420)
				break;
		}

		m_wTray = ROOT_AE3_UI.Text(m_wTaskbar, m_fW - 400, 11, 260, 20, m_sTray, 14, ROOT_AE3_Theme.Text());
		m_wClock = ROOT_AE3_UI.Text(m_wTaskbar, m_fW - 130, 11, 124, 20, "", 14, ROOT_AE3_Theme.Text());
		UpdateClock();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateClock()
	{
		if (m_wClock)
		{
			string now = ROOT_AE3_Util.Now();
			m_wClock.SetText(now.Substring(0, 16));
		}

		if (m_wTray)
			m_wTray.SetText(m_sTray);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleMenu()
	{
		if (m_wMenu)
		{
			m_wMenu.RemoveFromHierarchy();
			m_wMenu = null;
			return;
		}

		bool files = ROOT_AE3_SettingsManager.Get().m_bDesktopEnableFileBrowsing;
		array<int> idx = {};
		array<string> names = {};
		foreach (int i, typename t : m_aApps)
		{
			ROOT_AE3_App app = ROOT_AE3_App.Cast(t.Spawn());
			if (app && app.IsListed() && (!app.NeedsFileBrowsing() || files))
			{
				idx.Insert(i);
				names.Insert(app.GetTitle());
			}
		}

		float rowH = 26;
		float h = (idx.Count() + 4) * rowH + 16;
		float w = 240;
		m_MenuListener = new ROOT_AE3_ForwardListener(this);
		m_wMenu = ROOT_AE3_UI.Frame(m_wRoot, 4, m_fH - BAR_H - h - 4, w, h);
		m_wMenu.SetZOrder(100001);
		ROOT_AE3_UI.Rect(m_wMenu, 0, 0, w, h, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.Text(m_wMenu, 10, 6, w - 20, 20, ROOT_AE3_Client.Get().m_sUser + "@" + ROOT_AE3_Client.Get().m_sHostname, 14, ROOT_AE3_Theme.Muted());
		float y = 30;
		foreach (int k, int i : idx)
		{
			ROOT_AE3_UI.Button(m_wMenu, 6, y, w - 12, rowH - 2, names[k], ID_MENU + i, m_MenuListener, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ROOT_AE3_Theme.Text(), 15);
			y += rowH;
		}

		ROOT_AE3_UI.Button(m_wMenu, 6, y, w - 12, rowH - 2, "Sign out", ID_SIGNOUT, m_MenuListener, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ROOT_AE3_Theme.Text(), 15);
		y += rowH;
		ROOT_AE3_UI.Button(m_wMenu, 6, y, w - 12, rowH - 2, "Standby", ID_STANDBY, m_MenuListener, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ROOT_AE3_Theme.Text(), 15);
		y += rowH;
		ROOT_AE3_UI.Button(m_wMenu, 6, y, w - 12, rowH - 2, "Shut down", ID_SHUTDOWN, m_MenuListener, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ROOT_AE3_Theme.Text(), 15);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens an app; an open single-instance app is focused instead.
	ROOT_AE3_App OpenApp(typename t, string arg = "")
	{
		if (m_wMenu)
			ToggleMenu();

		if (!LoggedIn())
			return null;

		foreach (ROOT_AE3_Window w : m_aWindows)
		{
			if (w.m_App.Type() == t && arg.IsEmpty())
			{
				w.SetMinimized(false);
				RefreshTaskbar();
				return w.m_App;
			}
		}

		ROOT_AE3_App app = ROOT_AE3_App.Cast(t.Spawn());
		if (!app)
			return null;

		app.m_sArg = arg;
		int slot = m_aWindows.Count() % 8;
		float offset = slot * 28;
		vector size = app.GetSize();
		float x = Math.Max(0, Math.Min(140 + offset, m_fW - size[0]));
		float y = Math.Max(0, Math.Min(60 + offset, m_fH - BAR_H - size[1]));
		ROOT_AE3_Window win = new ROOT_AE3_Window(this, app, x, y);
		m_aWindows.Insert(win);
		win.BringToFront();
		RefreshTaskbar();
		PushMirror();
		return app;
	}

	//------------------------------------------------------------------------------------------------
	void CloseWindow(ROOT_AE3_Window w)
	{
		w.Destroy();
		m_aWindows.RemoveItem(w);
		if (m_Focus == w)
			m_Focus = null;

		RefreshTaskbar();
		PushMirror();
	}

	//------------------------------------------------------------------------------------------------
	//! Esc: closes the start menu when open.
	bool HandleBack()
	{
		if (m_wMenu)
		{
			ToggleMenu();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the window list for the device screen mirror.
	protected void PushMirror()
	{
		string s = "armaOS desktop - " + ROOT_AE3_Client.Get().m_sUser;
		foreach (ROOT_AE3_Window w : m_aWindows)
		{
			s += "\n[ " + w.m_App.GetTitle() + " ]";
		}

		array<string> args = {s};
		ROOT_AE3_Client.Get().Request("ui_mirror", args, null);
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt)
	{
		foreach (ROOT_AE3_Window w : m_aWindows)
		{
			w.Update(dt);
		}

		m_fClockTimer += dt;
		if (m_fClockTimer > 5)
		{
			m_fClockTimer = 0;
			UpdateClock();
			if (LoggedIn())
			{
				array<string> args = {};
				ROOT_AE3_Client.Get().Request("tray", args, this);
			}
		}

		if (m_wToast)
		{
			m_fToastTimer -= dt;
			if (m_fToastTimer <= 0)
			{
				m_wToast.RemoveFromHierarchy();
				m_wToast = null;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void Toast(string title, string text)
	{
		if (m_wToast)
			m_wToast.RemoveFromHierarchy();

		float w = 340;
		m_wToast = ROOT_AE3_UI.Frame(m_wRoot, m_fW - w - 12, m_fH - BAR_H - 100, w, 88);
		m_wToast.SetZOrder(100002);
		ROOT_AE3_UI.Rect(m_wToast, 0, 0, w, 88, ROOT_AE3_Theme.Panel(), ID_TOAST, this);
		ROOT_AE3_UI.Rect(m_wToast, 0, 0, 4, 88, ROOT_AE3_Theme.Accent());
		ROOT_AE3_UI.Text(m_wToast, 14, 8, w - 24, 22, title, 16, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Text(m_wToast, 14, 32, w - 24, 50, text, 14, ROOT_AE3_Theme.Muted(), false, true);
		m_fToastTimer = 6;
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.HINT);
	}

	//------------------------------------------------------------------------------------------------
	void OnTerminalChanged()
	{
		foreach (ROOT_AE3_Window w : m_aWindows)
		{
			ROOT_AE3_AppTerminal t = ROOT_AE3_AppTerminal.Cast(w.m_App);
			if (t)
				t.Refresh();
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnUserChanged()
	{
		if (m_bShown)
			Build();
	}

	//------------------------------------------------------------------------------------------------
	void OnGameState(string data)
	{
		array<string> f = ROOT_AE3_Codec.Split(data);
		if (f.Count() < 3)
			return;

		foreach (ROOT_AE3_Window w : m_aWindows)
		{
			ROOT_AE3_AppGames g = ROOT_AE3_AppGames.Cast(w.m_App);
			if (g)
				g.OnState(f[0], f[1].ToInt(), f[2]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Re-reads theme and wallpaper and rebuilds.
	void ApplyPrefs(string theme, string wallpaper)
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		c.m_sTheme = theme;
		c.m_sWallpaper = wallpaper;
		Build();
	}

	//------------------------------------------------------------------------------------------------
	protected void Login()
	{
		if (!m_wUser || !m_wPass)
			return;

		array<string> args = {m_wUser.GetText(), m_wPass.GetText()};
		ROOT_AE3_Client.Get().Request("login", args, this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (cmd == "login")
		{
			if (!ok && m_wLoginMsg)
				m_wLoginMsg.SetText(ROOT_AE3_App.Arg(payload, 0));

			return;
		}

		if (cmd == "tray" && ok)
		{
			m_sTray = ROOT_AE3_App.Arg(payload, 0);
			UpdateClock();
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (!finished)
			return false;

		if (id == ID_USER && m_wPass)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_wPass);
			m_wPass.ActivateWriteMode();
			return true;
		}

		if (id == ID_PASS)
		{
			Login();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseUp(Widget w, int id, int button)
	{
		foreach (ROOT_AE3_Window win : m_aWindows)
		{
			win.StopDrag();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiDoubleClick(Widget w, int id)
	{
		if (id >= ID_ICON && id < ID_ICON + m_aApps.Count())
		{
			OpenApp(m_aApps[id - ID_ICON]);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		if (id >= ID_MENU && id < ID_MENU + m_aApps.Count())
		{
			OpenApp(m_aApps[id - ID_MENU]);
			return true;
		}

		if (id >= ID_TASK && id < ID_TASK + m_aWindows.Count())
		{
			ROOT_AE3_Window win = m_aWindows[id - ID_TASK];
			if (win.IsMinimized() || win != m_Focus)
				win.SetMinimized(false);
			else
				win.SetMinimized(true);

			RefreshTaskbar();
			return true;
		}

		array<string> none = {};
		switch (id)
		{
			case ID_BG:
			{
				if (m_wMenu)
					ToggleMenu();

				return false;
			}

			case ID_START: ToggleMenu(); return true;
			case ID_LOGIN: Login(); return true;
			case ID_TERMINAL: c.Send(ROOT_AE3_EMsg.TERM_KEY, "MODE:0"); return true;
			case ID_SIGNOUT: c.Request("signout", none, this); return true;
			case ID_STANDBY: c.Request("standby", none, this); return true;
			case ID_SHUTDOWN: c.Request("shutdown", none, this); return true;
			case ID_TOAST:
			{
				if (m_wToast)
				{
					m_wToast.RemoveFromHierarchy();
					m_wToast = null;
				}

				return true;
			}
		}

		return false;
	}
}
