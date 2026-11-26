//! armaOS desktop: sign-in screen, wallpaper, icons, windows, taskbar, start menu with search, context menus,
//! dialogs, notifications and keyboard shortcuts.
class ROOT_AE3_DesktopView : ROOT_AE3_GuiReplyHandler
{
	protected static const int ID_BG = 1;
	protected static const int ID_START = 2;
	protected static const int ID_LOGIN = 3;
	protected static const int ID_USER = 4;
	protected static const int ID_PASS = 5;
	protected static const int ID_TERMINAL = 9;
	protected static const int ID_TOAST = 10;
	protected static const int ID_POWER = 11;
	protected static const int ID_SEARCH = 12;
	protected static const int ID_POPUP_BG = 13;
	protected static const int ID_DLG_OK = 14;
	protected static const int ID_DLG_CANCEL = 15;
	protected static const int ID_DLG_EDIT = 16;
	protected static const int ID_DLG_BG = 17;
	protected static const int ID_MENU_BG = 18;
	protected static const int ID_ICON = 100;
	protected static const int ID_MENU = 300;
	protected static const int ID_POPUP = 400;
	protected static const int ID_TASK = 500;
	protected static const int ID_PIN = 600;
	protected static const int MI_OPEN = 1;
	protected static const int MI_REFRESH = 2;
	protected static const int MI_PERSONALIZE = 3;
	protected static const int MI_TERMINAL = 4;
	protected static const int MI_FILES = 5;
	protected static const int MI_SIGNOUT = 6;
	protected static const int MI_STANDBY = 7;
	protected static const int MI_SHUTDOWN = 8;
	protected static const int MI_SYSINFO = 9;
	protected static const int MI_CLOSE = 10;
	protected static const int MI_MINIMIZE = 11;
	protected static const int MI_NEW_FOLDER = 12;
	protected static const int MI_NEW_FILE = 13;
	protected static const int MI_FILE_OPEN = 14;
	protected static const int MI_FILE_RENAME = 15;
	protected static const int MI_FILE_DELETE = 16;
	protected static const int ID_FILE = 2000;
	protected static const float BAR_H = 48;
	protected static const float CELL_W = 90;
	protected static const float CELL_H = 96;

	protected FrameWidget m_wRoot;
	protected FrameWidget m_wIcons;
	protected FrameWidget m_wWindows;
	protected FrameWidget m_wTaskbar;
	protected FrameWidget m_wMenu;
	protected FrameWidget m_wMenuGrid;
	protected FrameWidget m_wPopup;
	protected FrameWidget m_wDialog;
	protected FrameWidget m_wToast;
	protected TextWidget m_wClock;
	protected TextWidget m_wDate;
	protected TextWidget m_wTray;
	protected EditBoxWidget m_wUser;
	protected EditBoxWidget m_wPass;
	protected EditBoxWidget m_wSearch;
	protected EditBoxWidget m_wDlgEdit;
	protected TextWidget m_wLoginMsg;
	protected ref array<ref ROOT_AE3_Window> m_aWindows = {};
	protected ref array<typename> m_aApps = {};
	protected ref array<int> m_aMenuApps = {};
	protected ref array<int> m_aPopupIds = {};
	protected ROOT_AE3_UiListener m_PopupTarget;
	protected ROOT_AE3_UiListener m_DialogTarget;
	protected string m_sDialogTag;
	protected ROOT_AE3_Window m_Focus;
	protected ROOT_AE3_Window m_ContextWindow;
	protected int m_iZ = 10;
	protected int m_iSelIcon = -1;
	protected int m_iSelFile = -1;
	protected ref array<ref array<string>> m_aDeskFiles = {};	//!< ~/Desktop entries (fs_list fields)
	protected ImageWidget m_wWall;
	protected float m_fW, m_fH;
	protected float m_fGridW, m_fGridH;
	protected bool m_bShown;
	protected bool m_bWasTyping;
	protected float m_fClockTimer;
	protected float m_fToastTimer;
	protected string m_sTray;
	protected string m_sBuiltKey = "-";
	protected ref ROOT_AE3_ForwardListener m_IconListener;
	protected ref ROOT_AE3_ForwardListener m_TaskbarListener;
	protected ref ROOT_AE3_ForwardListener m_MenuListener;
	protected ref ROOT_AE3_ForwardListener m_MenuGridListener;
	protected ref ROOT_AE3_ForwardListener m_PopupListener;
	protected ref ROOT_AE3_ForwardListener m_DialogListener;
	protected ref ROOT_AE3_ForwardListener m_ToastListener;
	protected ref ROOT_AE3_FilePicker m_Picker;
	protected static ref array<KeyCode> s_aKeys = {
		KeyCode.KC_RETURN, KeyCode.KC_NUMPADENTER, KeyCode.KC_DELETE, KeyCode.KC_F2, KeyCode.KC_F5, KeyCode.KC_BACK, KeyCode.KC_SPACE,
		KeyCode.KC_UP, KeyCode.KC_DOWN, KeyCode.KC_LEFT, KeyCode.KC_RIGHT, KeyCode.KC_A, KeyCode.KC_C, KeyCode.KC_D, KeyCode.KC_S, KeyCode.KC_X, KeyCode.KC_V, KeyCode.KC_W, KeyCode.KC_TAB,
		KeyCode.KC_0, KeyCode.KC_1, KeyCode.KC_2, KeyCode.KC_3, KeyCode.KC_4, KeyCode.KC_5, KeyCode.KC_6, KeyCode.KC_7, KeyCode.KC_8, KeyCode.KC_9,
		KeyCode.KC_NUMPAD0, KeyCode.KC_NUMPAD1, KeyCode.KC_NUMPAD2, KeyCode.KC_NUMPAD3, KeyCode.KC_NUMPAD4, KeyCode.KC_NUMPAD5, KeyCode.KC_NUMPAD6, KeyCode.KC_NUMPAD7, KeyCode.KC_NUMPAD8, KeyCode.KC_NUMPAD9,
		KeyCode.KC_ADD, KeyCode.KC_SUBTRACT, KeyCode.KC_MULTIPLY, KeyCode.KC_DIVIDE, KeyCode.KC_DECIMAL, KeyCode.KC_PERIOD, KeyCode.KC_COMMA, KeyCode.KC_MINUS, KeyCode.KC_EQUALS, KeyCode.KC_SLASH
	};

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
		ClosePicker();
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
	//! Window area (desktop without the taskbar).
	void GetArea(out float w, out float h)
	{
		w = m_fW;
		h = m_fH - BAR_H;
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse position relative to the desktop, in reference units.
	void MouseLocal(out float x, out float y)
	{
		float mx, my, rx, ry;
		ROOT_AE3_UI.Mouse(mx, my);
		m_wRoot.GetScreenPos(rx, ry);
		WorkspaceWidget ws = GetGame().GetWorkspace();
		x = mx - ws.DPIUnscale(rx);
		y = my - ws.DPIUnscale(ry);
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
		if (m_Focus == w)
			return;

		m_Focus = w;
		RefreshTaskbar();
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_Window GetFocusWindow()
	{
		return m_Focus;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoggedIn()
	{
		return !ROOT_AE3_Client.Get().m_sUser.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected bool AppAllowed(ROOT_AE3_App app)
	{
		return app && app.IsListed() && (!app.NeedsFileBrowsing() || ROOT_AE3_SettingsManager.Get().m_bDesktopEnableFileBrowsing);
	}

	//------------------------------------------------------------------------------------------------
	//! Full rebuild (theme, user or wallpaper changed). Open windows are closed.
	void Build()
	{
		ClosePicker();
		m_sBuiltKey = BuildKey();
		foreach (ROOT_AE3_Window w : m_aWindows)
		{
			w.Destroy();
		}

		m_aWindows.Clear();
		m_Focus = null;
		ReleaseHandlers();
		ROOT_AE3_UI.Clear(m_wRoot);
		m_wMenu = null;
		m_wPopup = null;
		m_wDialog = null;
		m_wToast = null;
		m_wTaskbar = null;
		m_iSelIcon = -1;
		m_iSelFile = -1;
		m_aDeskFiles.Clear();
		ROOT_AE3_Theme.s_iTheme = ROOT_AE3_Client.Get().m_sTheme.ToInt();
		ROOT_AE3_UI.Rect(m_wRoot, 0, 0, m_fW, m_fH, ROOT_AE3_UI.Rgb(0, 0, 0), ID_BG, this);
		BuildWallpaper(ROOT_AE3_Client.Get().m_sWallpaper);

		if (!LoggedIn())
		{
			BuildLogin();
			return;
		}

		m_wIcons = ROOT_AE3_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH - BAR_H);
		BuildIcons();
		RequestDesktopFiles();
		m_wWindows = ROOT_AE3_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH - BAR_H, false);
		m_wTaskbar = ROOT_AE3_UI.Frame(m_wRoot, 0, m_fH - BAR_H, m_fW, BAR_H);
		m_wTaskbar.SetZOrder(100000);
		RefreshTaskbar();
		PushMirror();
	}

	//------------------------------------------------------------------------------------------------
	//! Wallpaper pref: 0 black, 1-5 built in, an image file path or a web address.
	protected void BuildWallpaper(string pref)
	{
		m_wWall = null;
		if (pref.StartsWith("/"))
		{
			m_wWall = ROOT_AE3_UI.Picture(m_wRoot, 0, 0, m_fW, m_fH, "");
			m_wWall.SetVisible(false);
			array<string> args = {pref};
			ROOT_AE3_Client.Get().Request("fs_media", args, this);
			return;
		}

		if (ROOT_AE3_WebMedia.KindOf(pref) == "url")
		{
			m_wWall = ROOT_AE3_UI.Picture(m_wRoot, 0, 0, m_fW, m_fH, "");
			m_wWall.SetVisible(false);
			ROOT_AE3_WebMedia.Request("url", pref, this);
			return;
		}

		ResourceName wall = ROOT_AE3_AppRegistry.Wallpaper(pref.ToInt());
		if (!wall.IsEmpty())
			ROOT_AE3_UI.Picture(m_wRoot, 0, 0, m_fW, m_fH, wall);
	}

	//------------------------------------------------------------------------------------------------
	//! Shows the custom wallpaper; file is a local image (web) or a texture resource.
	protected void ShowWallpaper(string file, bool local)
	{
		if (!m_wWall)
			return;

		bool loaded;
		if (local)
			loaded = m_wWall.LoadImageTexture(0, file, true, true);
		else
			loaded = m_wWall.LoadImageTexture(0, file);

		m_wWall.SetVisible(loaded);
		if (!loaded)
			Toast("armaOS", "The background image could not be displayed.");
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildLogin()
	{
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		ROOT_AE3_UI.Rect(m_wRoot, 0, 0, m_fW, m_fH, ROOT_AE3_UI.Rgb(0, 0, 0, 0.4));
		float w = 320;
		float x = (m_fW - w) * 0.5;
		float y = m_fH * 0.5 - 190;
		Color white = ROOT_AE3_UI.Rgb(1, 1, 1);
		ROOT_AE3_UI.Icon(m_wRoot, (m_fW - 128) * 0.5, y, 128, "user");
		TextWidget host = ROOT_AE3_UI.Text(m_wRoot, 0, y + 142, m_fW, 36, c.m_sHostname, 28, white, false, false, 1);
		host.SetShadow(4, 0xFF000000, 0.5, 0, 1);
		y += 196;
		ROOT_AE3_UI.Box(m_wRoot, x, y, w, 36, ROOT_AE3_UI.Rgb(1, 1, 1, 0.92), ROOT_AE3_UI.Rgb(1, 1, 1, 0.5));
		m_wUser = ROOT_AE3_UI.Edit(m_wRoot, x + 10, y + 5, w - 20, 26, "", ID_USER, this);
		ROOT_AE3_UI.Placeholder(m_wUser, m_wRoot, x + 10, y + 9, w - 20, "User name", ROOT_AE3_UI.Rgb(0.4, 0.4, 0.42));
		y += 46;
		ROOT_AE3_UI.Box(m_wRoot, x, y, w, 36, ROOT_AE3_UI.Rgb(1, 1, 1, 0.92), ROOT_AE3_UI.Rgb(1, 1, 1, 0.5));
		m_wPass = ROOT_AE3_UI.Edit(m_wRoot, x + 10, y + 5, w - 20, 26, "", ID_PASS, this, true);
		ROOT_AE3_UI.Placeholder(m_wPass, m_wRoot, x + 10, y + 9, w - 20, "Password", ROOT_AE3_UI.Rgb(0.4, 0.4, 0.42));
		if (m_wUser)
			m_wUser.SetColor(ROOT_AE3_UI.Rgb(0.1, 0.1, 0.1));

		if (m_wPass)
			m_wPass.SetColor(ROOT_AE3_UI.Rgb(0.1, 0.1, 0.1));

		y += 52;
		ROOT_AE3_UI.Button(m_wRoot, x, y, w, 36, "Sign in", ID_LOGIN, this, ROOT_AE3_Theme.Accent(), white, 16);
		y += 48;
		if (c.m_iInterface != 1)
			ROOT_AE3_UI.Button(m_wRoot, x + 60, y, w - 120, 30, "Use the terminal", ID_TERMINAL, this, ROOT_AE3_UI.Rgb(0, 0, 0, 0), white, 14, ROOT_AE3_UI.Rgb(1, 1, 1, 0.15));

		m_wLoginMsg = ROOT_AE3_UI.Text(m_wRoot, x - 40, y + 40, w + 80, 44, "", 15, ROOT_AE3_UI.Rgb(1, 0.75, 0.7), false, true, 1);
		m_wLoginMsg.SetShadow(4, 0xFF000000, 0.6, 0, 1);
		ROOT_AE3_UI.Text(m_wRoot, 0, m_fH - 34, m_fW - 24, 20, "Tab: next field   Enter: sign in", 13, ROOT_AE3_UI.Rgb(1, 1, 1, 0.7), false, false, 2);
		if (m_wUser)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_wUser);
			m_wUser.ActivateWriteMode();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildIcons()
	{
		ROOT_AE3_UI.Clear(m_wIcons);
		m_IconListener = new ROOT_AE3_ForwardListener(this);
		int col, row;
		int perCol = Math.Max(1, Math.Floor((m_fH - BAR_H - 16) / CELL_H));
		foreach (int i, typename t : m_aApps)
		{
			ROOT_AE3_App app = ROOT_AE3_App.Cast(t.Spawn());
			if (!AppAllowed(app) || !app.IsOnDesktop())
				continue;

			float x = 8 + col * CELL_W;
			float y = 8 + row * CELL_H;
			Color bg = ROOT_AE3_UI.Rgb(0, 0, 0, 0);
			if (i == m_iSelIcon)
				bg = ROOT_AE3_UI.Rgb(0.6, 0.8, 1, 0.3);

			ROOT_AE3_UI.Hit(m_wIcons, x, y, CELL_W - 4, CELL_H - 4, ID_ICON + i, m_IconListener, bg, ROOT_AE3_UI.Rgb(1, 1, 1, 0.16));
			ROOT_AE3_UI.Icon(m_wIcons, x + (CELL_W - 4 - 48) * 0.5, y + 6, 48, app.GetIcon());
			TextWidget label = ROOT_AE3_UI.Text(m_wIcons, x + 2, y + 58, CELL_W - 8, 34, app.GetTitle(), 13, ROOT_AE3_UI.Rgb(1, 1, 1), false, true, 1);
			label.SetShadow(3, 0xFF000000, 0.9, 0, 1);
			row++;
			if (row >= perCol)
			{
				row = 0;
				col++;
			}
		}

		foreach (int fi, array<string> e : m_aDeskFiles)
		{
			float fx = 8 + col * CELL_W;
			float fy = 8 + row * CELL_H;
			Color fbg = ROOT_AE3_UI.Rgb(0, 0, 0, 0);
			if (fi == m_iSelFile)
				fbg = ROOT_AE3_UI.Rgb(0.6, 0.8, 1, 0.3);

			ROOT_AE3_UI.Hit(m_wIcons, fx, fy, CELL_W - 4, CELL_H - 4, ID_FILE + fi, m_IconListener, fbg, ROOT_AE3_UI.Rgb(1, 1, 1, 0.16));
			ROOT_AE3_UI.Icon(m_wIcons, fx + (CELL_W - 4 - 48) * 0.5, fy + 6, 48, ROOT_AE3_Icons.ForFile(e[1], e[5] == "1", e[6] == "1", e[7] == "1", ROOT_AE3_Filesystem.BaseName(e[0])));
			TextWidget flabel = ROOT_AE3_UI.Text(m_wIcons, fx + 2, fy + 58, CELL_W - 8, 34, ROOT_AE3_Filesystem.BaseName(e[0]), 13, ROOT_AE3_UI.Rgb(1, 1, 1), false, true, 1);
			flabel.SetShadow(3, 0xFF000000, 0.9, 0, 1);
			row++;
			if (row >= perCol)
			{
				row = 0;
				col++;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop files come from ~/Desktop (file browsing setting on).
	protected bool DesktopFiles()
	{
		return LoggedIn() && ROOT_AE3_SettingsManager.Get().m_bDesktopEnableFileBrowsing;
	}

	//------------------------------------------------------------------------------------------------
	protected string DesktopDir()
	{
		return ROOT_AE3_Filesystem.HomeOf(ROOT_AE3_Client.Get().m_sUser) + "/Desktop";
	}

	//------------------------------------------------------------------------------------------------
	void RequestDesktopFiles()
	{
		if (!DesktopFiles())
			return;

		array<string> args = {DesktopDir()};
		ROOT_AE3_Client.Get().Request("fs_list", args, this);
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectFile(int index)
	{
		if (m_iSelFile == index && m_iSelIcon == -1)
			return;

		m_iSelFile = index;
		m_iSelIcon = -1;
		BuildIcons();
	}

	//------------------------------------------------------------------------------------------------
	//! Folders open in Files, images in Photos, programs run in the terminal, other files in Notepad.
	protected void OpenDeskFile(int index)
	{
		if (!m_aDeskFiles.IsIndexValid(index))
			return;

		array<string> e = m_aDeskFiles[index];
		if (e[1] == "dir")
		{
			OpenApp(ROOT_AE3_AppFiles, e[0]);
			return;
		}

		if (e[5] == "1")
		{
			OpenApp(ROOT_AE3_AppMedia, e[0]);
			return;
		}

		if (e[6] == "1")
		{
			OpenApp(ROOT_AE3_AppFiles, DesktopDir());
			return;
		}

		if (e[7] == "1")
		{
			array<string> args = {e[0]};
			ROOT_AE3_Client.Get().Request("run_file", args, this);
			return;
		}

		OpenApp(ROOT_AE3_AppNotepad, e[0]);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (tag == ROOT_AE3_WebMedia.TAG)
		{
			if (ok)
				ShowWallpaper(value, true);
			else
				Toast("armaOS", value);

			return;
		}

		value = value.Trim();
		if (!ok || value.IsEmpty())
			return;

		if (!ROOT_AE3_Filesystem.ValidName(value))
		{
			Toast("armaOS", "That name is not allowed.");
			return;
		}

		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		string path = DesktopDir() + "/" + value;
		switch (tag)
		{
			case "desk_newfolder":
			{
				array<string> a = {path};
				c.Request("fs_mkdir", a, this);
				return;
			}

			case "desk_newfile":
			{
				array<string> b = {path, ""};
				c.Request("fs_save", b, this);
				return;
			}

			case "desk_rename":
			{
				if (!m_aDeskFiles.IsIndexValid(m_iSelFile))
					return;

				array<string> m = {m_aDeskFiles[m_iSelFile][0], path};
				c.Request("fs_move", m, this);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectIcon(int index)
	{
		if (m_iSelIcon == index && m_iSelFile == -1)
			return;

		m_iSelIcon = index;
		m_iSelFile = -1;
		BuildIcons();
	}

	//------------------------------------------------------------------------------------------------
	//! Apps pinned to the taskbar.
	protected void GetPinned(notnull array<typename> pinned)
	{
		array<typename> want = {ROOT_AE3_AppFiles, ROOT_AE3_AppBrowser, ROOT_AE3_AppMail, ROOT_AE3_AppTerminal};
		foreach (typename t : want)
		{
			if (m_aApps.Contains(t) && AppAllowed(ROOT_AE3_App.Cast(t.Spawn())))
				pinned.Insert(t);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_Window FirstWindowOf(typename t)
	{
		foreach (ROOT_AE3_Window w : m_aWindows)
		{
			if (w.m_App.Type() == t)
				return w;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void TaskButton(float x, ROOT_AE3_Window w, string icon, int id)
	{
		Color bg = ROOT_AE3_UI.Rgb(0, 0, 0, 0);
		if (w && w == m_Focus)
			bg = ROOT_AE3_Theme.Hover();

		ROOT_AE3_UI.IconButton(m_wTaskbar, x, 4, 44, BAR_H - 8, icon, id, m_TaskbarListener, 26, null, bg);
		if (!w)
			return;

		float iw = 6;
		Color ic = ROOT_AE3_Theme.Muted();
		if (w == m_Focus)
		{
			iw = 16;
			ic = ROOT_AE3_Theme.Accent();
		}

		ROOT_AE3_UI.Rect(m_wTaskbar, x + (44 - iw) * 0.5, BAR_H - 6, iw, 3, ic);
	}

	//------------------------------------------------------------------------------------------------
	void RefreshTaskbar()
	{
		if (!m_wTaskbar)
			return;

		ROOT_AE3_UI.Clear(m_wTaskbar);
		m_TaskbarListener = new ROOT_AE3_ForwardListener(this);
		ROOT_AE3_UI.Rect(m_wTaskbar, 0, 0, m_fW, BAR_H, ROOT_AE3_Theme.Taskbar());
		ROOT_AE3_UI.Rect(m_wTaskbar, 0, 0, m_fW, 1, ROOT_AE3_Theme.Border());
		Color startBg = ROOT_AE3_UI.Rgb(0, 0, 0, 0);
		if (m_wMenu)
			startBg = ROOT_AE3_Theme.Hover();

		ROOT_AE3_UI.IconButton(m_wTaskbar, 6, 4, 44, BAR_H - 8, "logo", ID_START, m_TaskbarListener, 28, null, startBg);
		float x = 60;
		array<typename> pinned = {};
		GetPinned(pinned);
		array<ROOT_AE3_Window> shown = {};
		foreach (int k, typename t : pinned)
		{
			ROOT_AE3_Window w = FirstWindowOf(t);
			if (w)
			{
				shown.Insert(w);
				TaskButton(x, w, w.m_App.GetIcon(), ID_TASK + m_aWindows.Find(w));
			}
			else
			{
				ROOT_AE3_App app = ROOT_AE3_App.Cast(t.Spawn());
				TaskButton(x, null, app.GetIcon(), ID_PIN + m_aApps.Find(t));
			}

			x += 48;
		}

		foreach (int i, ROOT_AE3_Window w2 : m_aWindows)
		{
			if (shown.Contains(w2))
				continue;

			if (x > m_fW - 300)
				break;

			TaskButton(x, w2, w2.m_App.GetIcon(), ID_TASK + i);
			x += 48;
		}

		m_wTray = ROOT_AE3_UI.Text(m_wTaskbar, m_fW - 420, 16, 290, 18, m_sTray, 13, ROOT_AE3_Theme.Muted(), false, false, 2);
		m_wClock = ROOT_AE3_UI.Text(m_wTaskbar, m_fW - 120, 6, 108, 18, "", 14, ROOT_AE3_Theme.Text(), false, false, 2);
		m_wDate = ROOT_AE3_UI.Text(m_wTaskbar, m_fW - 120, 25, 108, 18, "", 12, ROOT_AE3_Theme.Text(), false, false, 2);
		UpdateClock();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateClock()
	{
		string now = ROOT_AE3_Util.Now();
		if (m_wClock && now.Length() >= 16)
		{
			m_wClock.SetText(now.Substring(11, 5));
			m_wDate.SetText(now.Substring(0, 10));
		}

		if (m_wTray)
			m_wTray.SetText(m_sTray);
	}

	// Start menu ------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void ToggleMenu()
	{
		if (m_wMenu)
		{
			CloseStart();
			return;
		}

		float w = Math.Min(620, m_fW - 12);
		float h = Math.Min(560, m_fH - BAR_H - 12);
		float x = 6;
		float y = m_fH - BAR_H - h - 6;
		m_MenuListener = new ROOT_AE3_ForwardListener(this);
		m_wMenu = ROOT_AE3_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH - BAR_H, false);
		m_wMenu.SetZOrder(100001);
		ROOT_AE3_UI.Rect(m_wMenu, 0, 0, m_fW, m_fH - BAR_H, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ID_MENU_BG, m_MenuListener);
		ROOT_AE3_UI.Shadow(m_wMenu, x, y, w, h, 14);
		ROOT_AE3_UI.Box(m_wMenu, x, y, w, h, ROOT_AE3_Theme.Panel());
		// Swallows clicks on the panel background.
		ROOT_AE3_UI.Rect(m_wMenu, x + 1, y + 1, w - 2, h - 2, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ID_START + 1000, m_MenuListener);
		ROOT_AE3_UI.Box(m_wMenu, x + 24, y + 22, w - 48, 36, ROOT_AE3_Theme.Field());
		ROOT_AE3_UI.Icon(m_wMenu, x + 36, y + 32, 16, "g_search", ROOT_AE3_Theme.Muted());
		m_wSearch = ROOT_AE3_UI.Edit(m_wMenu, x + 62, y + 27, w - 100, 26, "", ID_SEARCH, m_MenuListener);
		if (m_wSearch)
			m_wSearch.SetColor(ROOT_AE3_Theme.Text());

		ROOT_AE3_UI.Placeholder(m_wSearch, m_wMenu, x + 62, y + 31, w - 100, "Type here to search", ROOT_AE3_Theme.Muted());
		ROOT_AE3_UI.Text(m_wMenu, x + 32, y + 72, 200, 20, "Apps", 15, ROOT_AE3_Theme.Text());
		m_fGridW = w - 40;
		m_fGridH = h - 100 - 72;
		m_wMenuGrid = ROOT_AE3_UI.Frame(m_wMenu, x + 20, y + 100, m_fGridW, m_fGridH);
		ROOT_AE3_UI.Rect(m_wMenu, x + 1, y + h - 64, w - 2, 63, ROOT_AE3_Theme.Bar());
		ROOT_AE3_UI.Rect(m_wMenu, x + 1, y + h - 65, w - 2, 1, ROOT_AE3_Theme.Border());
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		ROOT_AE3_UI.Icon(m_wMenu, x + 28, y + h - 48, 32, "user");
		ROOT_AE3_UI.Text(m_wMenu, x + 70, y + h - 50, 300, 18, c.m_sUser, 15, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Text(m_wMenu, x + 70, y + h - 30, 300, 16, c.m_sHostname, 12, ROOT_AE3_Theme.Muted());
		ROOT_AE3_UI.IconButton(m_wMenu, x + w - 64, y + h - 56, 44, 44, "g_power", ID_POWER, m_MenuListener, 20);
		FillMenuGrid("");
		RefreshTaskbar();
		if (m_wSearch)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_wSearch);
			m_wSearch.ActivateWriteMode();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseStart()
	{
		if (!m_wMenu)
			return;

		m_wMenu.RemoveFromHierarchy();
		m_wMenu = null;
		m_wSearch = null;
		GetGame().GetWorkspace().SetFocusedWidget(null);
		RefreshTaskbar();
	}

	//------------------------------------------------------------------------------------------------
	protected void FillMenuGrid(string filter)
	{
		if (!m_wMenuGrid)
			return;

		ROOT_AE3_UI.Clear(m_wMenuGrid);
		m_MenuGridListener = new ROOT_AE3_ForwardListener(this);
		m_aMenuApps.Clear();
		string f = filter.Trim();
		f.ToLower();
		float gw = m_fGridW;
		float gh = m_fGridH;
		int cols = Math.Max(1, Math.Floor(gw / 96));
		float cw = gw / cols;
		int maxRows = Math.Max(1, Math.Floor(gh / 86));
		foreach (int i, typename t : m_aApps)
		{
			ROOT_AE3_App app = ROOT_AE3_App.Cast(t.Spawn());
			if (!AppAllowed(app))
				continue;

			string title = app.GetTitle();
			string low = title;
			low.ToLower();
			if (!f.IsEmpty() && !low.Contains(f))
				continue;

			int k = m_aMenuApps.Count();
			if (k / cols >= maxRows)
				break;

			m_aMenuApps.Insert(i);
			int gx = k % cols;
			int gy = k / cols;
			float x = gx * cw;
			float y = gy * 86;
			ROOT_AE3_UI.Hit(m_wMenuGrid, x + 2, y, cw - 4, 82, ID_MENU + i, m_MenuGridListener, ROOT_AE3_UI.Rgb(0, 0, 0, 0));
			ROOT_AE3_UI.Icon(m_wMenuGrid, x + (cw - 36) * 0.5, y + 10, 36, app.GetIcon());
			ROOT_AE3_UI.Text(m_wMenuGrid, x + 4, y + 52, cw - 8, 30, title, 12, ROOT_AE3_Theme.Text(), false, true, 1);
		}

		if (m_aMenuApps.IsEmpty())
			ROOT_AE3_UI.Text(m_wMenuGrid, 0, 40, gw, 20, "No apps match your search.", 14, ROOT_AE3_Theme.Muted(), false, false, 1);
	}

	// Context menu ----------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Popup menu at the cursor (x / y below 0) or at x / y. Label "-" is a separator.
	//! The chosen id goes to target.OnMenuItem.
	void ShowMenu(ROOT_AE3_UiListener target, notnull array<string> labels, notnull array<int> ids, array<string> icons = null, float x = -1, float y = -1)
	{
		CloseMenu();
		float px = x;
		float py = y;
		if (px < 0 || py < 0)
			MouseLocal(px, py);

		float w = 230;
		float h = 8;
		foreach (string l : labels)
		{
			if (l == "-")
				h += 9;
			else
				h += 32;
		}

		if (px + w > m_fW - 4)
			px = Math.Max(4, px - w);

		if (py + h > m_fH - BAR_H - 4)
			py = Math.Max(4, py - h);

		m_PopupTarget = target;
		m_aPopupIds.Copy(ids);
		m_PopupListener = new ROOT_AE3_ForwardListener(this);
		m_wPopup = ROOT_AE3_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH, false);
		m_wPopup.SetZOrder(200000);
		ROOT_AE3_UI.Rect(m_wPopup, 0, 0, m_fW, m_fH, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ID_POPUP_BG, m_PopupListener);
		ROOT_AE3_UI.Shadow(m_wPopup, px, py, w, h, 8);
		ROOT_AE3_UI.Box(m_wPopup, px, py, w, h, ROOT_AE3_Theme.Panel());
		float ry = py + 4;
		foreach (int k, string label : labels)
		{
			if (label == "-")
			{
				ROOT_AE3_UI.Rect(m_wPopup, px + 8, ry + 4, w - 16, 1, ROOT_AE3_Theme.Border());
				ry += 9;
				continue;
			}

			string icon = "";
			if (icons && k < icons.Count())
				icon = icons[k];

			ROOT_AE3_UI.ItemButton(m_wPopup, px + 4, ry, w - 8, 32, icon, label, ID_POPUP + k, m_PopupListener, 14);
			ry += 32;
		}
	}

	//------------------------------------------------------------------------------------------------
	void CloseMenu()
	{
		if (m_wPopup)
			m_wPopup.RemoveFromHierarchy();

		m_wPopup = null;
	}

	// Dialog ----------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Modal dialog with an optional text field; the result goes to target.OnDialog(tag, ok, value).
	void ShowDialog(ROOT_AE3_UiListener target, string tag, string title, string message, bool input = false, string value = "", string okLabel = "OK", bool password = false)
	{
		CloseDialog();
		CloseMenu();
		m_DialogTarget = target;
		m_sDialogTag = tag;
		float w = 440;
		float h = 176;
		if (input)
			h += 44;

		float x = (m_fW - w) * 0.5;
		float y = (m_fH - BAR_H - h) * 0.45;
		m_DialogListener = new ROOT_AE3_ForwardListener(this);
		m_wDialog = ROOT_AE3_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH, false);
		m_wDialog.SetZOrder(150000);
		ROOT_AE3_UI.Rect(m_wDialog, 0, 0, m_fW, m_fH, ROOT_AE3_UI.Rgb(0, 0, 0, 0.25), ID_DLG_BG, m_DialogListener);
		ROOT_AE3_UI.Shadow(m_wDialog, x, y, w, h, 16);
		ROOT_AE3_UI.Box(m_wDialog, x, y, w, h, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.Text(m_wDialog, x + 24, y + 20, w - 48, 24, title, 18, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Text(m_wDialog, x + 24, y + 54, w - 48, 44, message, 14, ROOT_AE3_Theme.Text(), false, true);
		m_wDlgEdit = null;
		if (input)
		{
			ROOT_AE3_UI.Box(m_wDialog, x + 24, y + 104, w - 48, 32, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Accent());
			m_wDlgEdit = ROOT_AE3_UI.Edit(m_wDialog, x + 32, y + 107, w - 64, 26, value, ID_DLG_EDIT, m_DialogListener, password);
			if (m_wDlgEdit)
				m_wDlgEdit.SetColor(ROOT_AE3_Theme.Text());
		}

		ROOT_AE3_UI.Rect(m_wDialog, x + 1, y + h - 64, w - 2, 63, ROOT_AE3_Theme.Bar());
		ROOT_AE3_UI.Button(m_wDialog, x + w - 252, y + h - 48, 110, 32, okLabel, ID_DLG_OK, m_DialogListener, ROOT_AE3_Theme.Accent(), ROOT_AE3_UI.Rgb(1, 1, 1), 14);
		ROOT_AE3_UI.Rect(m_wDialog, x + w - 134, y + h - 48, 110, 32, ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.Button(m_wDialog, x + w - 133, y + h - 47, 108, 30, "Cancel", ID_DLG_CANCEL, m_DialogListener, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Text(), 14);
		if (m_wDlgEdit)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_wDlgEdit);
			m_wDlgEdit.ActivateWriteMode();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseDialog()
	{
		if (m_wDialog)
			m_wDialog.RemoveFromHierarchy();

		m_wDialog = null;
		m_wDlgEdit = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishDialog(bool ok)
	{
		if (!m_wDialog)
			return;

		string value;
		if (m_wDlgEdit)
			value = m_wDlgEdit.GetText();

		ROOT_AE3_UiListener target = m_DialogTarget;
		string tag = m_sDialogTag;
		CloseDialog();
		GetGame().GetWorkspace().SetFocusedWidget(null);
		if (target)
			target.OnDialog(tag, ok, value);
	}

	// File picker -----------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Modal Open / Save dialog; the chosen absolute path goes to target.OnDialog(tag, ok, path).
	//! start: folder shown first (empty = home). save: the name may be new. folders: pick a folder.
	void ShowFilePicker(ROOT_AE3_UiListener target, string tag, string title, string start = "", bool save = false, string fileName = "", string okLabel = "Open", bool folders = false)
	{
		ClosePicker();
		CloseMenu();
		CloseDialog();
		float aw, ah;
		GetArea(aw, ah);
		m_Picker = new ROOT_AE3_FilePicker(this, m_wRoot, aw, ah, target, tag, title, start, save, fileName, okLabel, folders);
	}

	//------------------------------------------------------------------------------------------------
	void ClosePicker()
	{
		if (m_Picker)
			m_Picker.Destroy();

		m_Picker = null;
	}

	// Windows ---------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Opens an app; an open single-instance app is focused instead.
	ROOT_AE3_App OpenApp(typename t, string arg = "")
	{
		CloseStart();
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
		float offset = slot * 30;
		vector size = app.GetSize();
		float aw, ah;
		GetArea(aw, ah);
		float x = Math.Max(0, Math.Min((aw - size[0]) * 0.5 - 60 + offset, aw - size[0]));
		float y = Math.Max(0, Math.Min((ah - size[1]) * 0.35 + offset, ah - size[1]));
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
	//! Esc: closes the context menu, dialog or start menu, in that order.
	bool HandleBack()
	{
		if (m_wPopup)
		{
			CloseMenu();
			return true;
		}

		if (m_wDialog)
		{
			FinishDialog(false);
			return true;
		}

		if (m_Picker)
		{
			m_Picker.Cancel();
			return true;
		}

		if (m_wMenu)
		{
			CloseStart();
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
		ROOT_AE3_UI.UpdatePlaceholders();
		UpdateKeys();
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
			if (m_fToastTimer < 0.3)
				m_wToast.SetOpacity(Math.Max(0, m_fToastTimer / 0.3));

			if (m_fToastTimer <= 0)
			{
				m_wToast.RemoveFromHierarchy();
				m_wToast = null;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Keys are read every frame; they act only while no edit box is in write mode (Tab on the sign-in screen always).
	protected void UpdateKeys()
	{
		bool typing = ROOT_AE3_UI.Typing();
		bool wasTyping = m_bWasTyping;
		m_bWasTyping = typing;
		bool ctrl = Debug.KeyState(KeyCode.KC_LCONTROL) != 0 || Debug.KeyState(KeyCode.KC_RCONTROL) != 0;
		foreach (KeyCode key : s_aKeys)
		{
			if (!ROOT_AE3_Keys.Pressed(key))
				continue;

			if (key == KeyCode.KC_TAB)
			{
				if (!LoggedIn())
					LoginTab();

				continue;
			}

			if (typing || wasTyping || !LoggedIn())
				continue;

			HandleKey(key, ctrl);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LoginTab()
	{
		if (!m_wUser || !m_wPass)
			return;

		EditBoxWidget next = m_wUser;
		if (m_wUser.IsInWriteMode() || GetGame().GetWorkspace().GetFocusedWidget() == m_wUser)
			next = m_wPass;

		GetGame().GetWorkspace().SetFocusedWidget(next);
		next.ActivateWriteMode();
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleKey(KeyCode key, bool ctrl)
	{
		bool enter = key == KeyCode.KC_RETURN || key == KeyCode.KC_NUMPADENTER;
		if (m_wDialog)
		{
			if (enter)
				FinishDialog(true);

			return;
		}

		if (m_wPopup || m_wMenu)
			return;

		if (m_Picker)
		{
			m_Picker.OnKey(key);
			return;
		}

		if (m_Focus && !m_Focus.IsMinimized())
		{
			if (ctrl && key == KeyCode.KC_W)
			{
				CloseWindow(m_Focus);
				return;
			}

			m_Focus.m_App.OnKey(key, ctrl);
			return;
		}

		if (enter && m_iSelIcon >= 0)
		{
			OpenApp(m_aApps[m_iSelIcon]);
			return;
		}

		if (enter && m_iSelFile >= 0)
		{
			OpenDeskFile(m_iSelFile);
			return;
		}

		if (key == KeyCode.KC_UP || key == KeyCode.KC_DOWN)
		{
			int dir = 1;
			if (key == KeyCode.KC_UP)
				dir = -1;

			MoveIconSelection(dir);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void MoveIconSelection(int dir)
	{
		array<int> shown = {};
		foreach (int i, typename t : m_aApps)
		{
			ROOT_AE3_App app = ROOT_AE3_App.Cast(t.Spawn());
			if (AppAllowed(app) && app.IsOnDesktop())
				shown.Insert(i);
		}

		if (shown.IsEmpty())
			return;

		int pos = shown.Find(m_iSelIcon);
		if (pos < 0)
			pos = 0;
		else
			pos = Math.ClampInt(pos + dir, 0, shown.Count() - 1);

		SelectIcon(shown[pos]);
	}

	//------------------------------------------------------------------------------------------------
	void Toast(string title, string text)
	{
		if (m_wToast)
			m_wToast.RemoveFromHierarchy();

		float w = 360;
		float h = 92;
		float x = m_fW - w - 12;
		float y = m_fH - BAR_H - h - 12;
		m_ToastListener = new ROOT_AE3_ForwardListener(this);
		m_wToast = ROOT_AE3_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH, false);
		m_wToast.SetZOrder(100002);
		ROOT_AE3_UI.Shadow(m_wToast, x, y, w, h, 10);
		ROOT_AE3_UI.Box(m_wToast, x, y, w, h, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.Rect(m_wToast, x + 1, y + 1, w - 2, h - 2, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ID_TOAST, m_ToastListener);
		ROOT_AE3_UI.Icon(m_wToast, x + 16, y + 16, 28, "logo");
		ROOT_AE3_UI.Text(m_wToast, x + 58, y + 14, w - 72, 20, title, 15, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Text(m_wToast, x + 58, y + 38, w - 72, 46, text, 13, ROOT_AE3_Theme.Muted(), false, true);
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

		if (m_wLoginMsg)
			m_wLoginMsg.SetText("Signing in...");

		array<string> args = {m_wUser.GetText(), m_wPass.GetText()};
		ROOT_AE3_Client.Get().Request("login", args, this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (cmd == "login")
		{
			if (!ok && m_wLoginMsg)
			{
				m_wLoginMsg.SetText(ROOT_AE3_App.Arg(payload, 0));
				if (m_wPass)
				{
					ROOT_AE3_UI.SetEdit(m_wPass, "");
					GetGame().GetWorkspace().SetFocusedWidget(m_wPass);
					m_wPass.ActivateWriteMode();
				}
			}

			return;
		}

		if (cmd == "tray" && ok)
		{
			m_sTray = ROOT_AE3_App.Arg(payload, 0);
			UpdateClock();
			return;
		}

		if (cmd == "fs_media")
		{
			if (!ok)
			{
				Toast("armaOS", "Background: " + ROOT_AE3_App.Arg(payload, 0));
				return;
			}

			string kind = ROOT_AE3_App.Arg(payload, 3);
			if (ROOT_AE3_WebMedia.IsWeb(kind))
				ROOT_AE3_WebMedia.Request(kind, ROOT_AE3_App.Arg(payload, 1), this);
			else
				ShowWallpaper(ROOT_AE3_App.Arg(payload, 1), false);

			return;
		}

		if (cmd == "fs_list")
		{
			if (!ok || payload.IsEmpty() || payload[0] != DesktopDir())
				return;

			m_aDeskFiles.Clear();
			for (int i = 1; i < payload.Count(); i++)
			{
				array<string> e = ROOT_AE3_Codec.SplitSub(payload[i]);
				if (e.Count() >= 8)
					m_aDeskFiles.Insert(e);
			}

			m_iSelFile = -1;
			if (m_wIcons)
				BuildIcons();

			return;
		}

		if (ok && (cmd == "fs_mkdir" || cmd == "fs_save" || cmd == "fs_move" || cmd == "fs_delete"))
		{
			RequestDesktopFiles();
			return;
		}

		if (!ok && !payload.IsEmpty())
			Toast("armaOS", payload[0]);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (id == ID_SEARCH)
		{
			if (!m_wSearch)
				return true;

			if (!finished)
			{
				FillMenuGrid(m_wSearch.GetText());
				return true;
			}

			if (!m_aMenuApps.IsEmpty())
				OpenApp(m_aApps[m_aMenuApps[0]]);

			return true;
		}

		if (id == ID_DLG_EDIT)
		{
			if (finished)
				FinishDialog(true);

			return true;
		}

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

		if (id >= ID_FILE && id < ID_FILE + m_aDeskFiles.Count())
		{
			OpenDeskFile(id - ID_FILE);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void BackgroundMenu()
	{
		array<string> labels = {"Refresh", "-", "Open Files", "Open Terminal", "-", "Personalize", "System info"};
		array<int> ids = {MI_REFRESH, 0, MI_FILES, MI_TERMINAL, 0, MI_PERSONALIZE, MI_SYSINFO};
		array<string> icons = {"g_refresh", "", "folder", "terminal", "", "settings", "info"};
		if (DesktopFiles())
		{
			labels.InsertAt("-", 1);
			ids.InsertAt(0, 1);
			icons.InsertAt("", 1);
			labels.InsertAt("New text document", 1);
			ids.InsertAt(MI_NEW_FILE, 1);
			icons.InsertAt("g_newfile", 1);
			labels.InsertAt("New folder", 1);
			ids.InsertAt(MI_NEW_FOLDER, 1);
			icons.InsertAt("g_newfolder", 1);
		}

		ShowMenu(this, labels, ids, icons);
	}

	//------------------------------------------------------------------------------------------------
	protected void TaskMenu(ROOT_AE3_Window win)
	{
		m_ContextWindow = win;
		array<string> labels = {"Minimize", "Close window"};
		array<int> ids = {MI_MINIMIZE, MI_CLOSE};
		array<string> icons = {"g_min", "g_close"};
		float mx, my;
		MouseLocal(mx, my);
		ShowMenu(this, labels, ids, icons, mx, m_fH - BAR_H - 4 - 8 - 64);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuItem(int id)
	{
		switch (id)
		{
			case MI_OPEN:
			{
				if (m_iSelIcon >= 0)
					OpenApp(m_aApps[m_iSelIcon]);

				return;
			}

			case MI_REFRESH:
			{
				BuildIcons();
				RequestDesktopFiles();
				return;
			}

			case MI_NEW_FOLDER: ShowDialog(this, "desk_newfolder", "New folder", "Name of the new folder on the desktop:", true, "New folder", "Create"); return;
			case MI_NEW_FILE: ShowDialog(this, "desk_newfile", "New text document", "Name of the new file on the desktop:", true, "New document.txt", "Create"); return;
			case MI_FILE_OPEN: OpenDeskFile(m_iSelFile); return;
			case MI_FILE_RENAME:
			{
				if (m_aDeskFiles.IsIndexValid(m_iSelFile))
				{
					string old = ROOT_AE3_Filesystem.BaseName(m_aDeskFiles[m_iSelFile][0]);
					ShowDialog(this, "desk_rename", "Rename", "New name for \"" + old + "\":", true, old, "Rename");
				}

				return;
			}

			case MI_FILE_DELETE:
			{
				if (m_aDeskFiles.IsIndexValid(m_iSelFile))
				{
					array<string> del = {m_aDeskFiles[m_iSelFile][0]};
					ROOT_AE3_Client.Get().Request("fs_delete", del, this);
				}

				return;
			}
			case MI_PERSONALIZE: OpenApp(ROOT_AE3_AppSettings); return;
			case MI_SYSINFO: OpenApp(ROOT_AE3_AppSysInfo); return;
			case MI_TERMINAL: OpenApp(ROOT_AE3_AppTerminal); return;
			case MI_FILES: OpenApp(ROOT_AE3_AppFiles); return;
			case MI_MINIMIZE:
			{
				if (m_ContextWindow && m_aWindows.Contains(m_ContextWindow))
				{
					m_ContextWindow.SetMinimized(true);
					RefreshTaskbar();
				}

				return;
			}

			case MI_CLOSE:
			{
				if (m_ContextWindow && m_aWindows.Contains(m_ContextWindow))
					CloseWindow(m_ContextWindow);

				return;
			}
		}

		array<string> none = {};
		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		switch (id)
		{
			case MI_SIGNOUT: c.Request("signout", none, this); return;
			case MI_STANDBY: c.Request("standby", none, this); return;
			case MI_SHUTDOWN: c.Request("shutdown", none, this); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == ID_POPUP_BG)
		{
			CloseMenu();
			return true;
		}

		if (id >= ID_POPUP && id < ID_POPUP + m_aPopupIds.Count())
		{
			int item = m_aPopupIds[id - ID_POPUP];
			ROOT_AE3_UiListener target = m_PopupTarget;
			CloseMenu();
			if (target)
				target.OnMenuItem(item);

			return true;
		}

		if (id == ID_DLG_BG)
			return true;

		if (id == ID_DLG_OK || id == ID_DLG_CANCEL)
		{
			FinishDialog(id == ID_DLG_OK);
			return true;
		}

		if (id == ID_MENU_BG)
		{
			CloseStart();
			return true;
		}

		if (id == ID_START + 1000)
			return true;

		if (id >= ID_MENU && id < ID_MENU + m_aApps.Count())
		{
			OpenApp(m_aApps[id - ID_MENU]);
			return true;
		}

		if (id >= ID_ICON && id < ID_ICON + m_aApps.Count())
		{
			SetFocusWindow(null);
			SelectIcon(id - ID_ICON);
			if (button == 1)
			{
				array<string> labels = {"Open"};
				array<int> ids = {MI_OPEN};
				array<string> icons = {"g_open"};
				ShowMenu(this, labels, ids, icons);
			}

			return true;
		}

		if (id >= ID_FILE && id < ID_FILE + m_aDeskFiles.Count())
		{
			SetFocusWindow(null);
			SelectFile(id - ID_FILE);
			if (button == 1)
			{
				array<string> flabels = {"Open", "Rename", "Delete"};
				array<int> fids = {MI_FILE_OPEN, MI_FILE_RENAME, MI_FILE_DELETE};
				array<string> ficons = {"g_open", "g_rename", "g_delete"};
				ShowMenu(this, flabels, fids, ficons);
			}

			return true;
		}

		if (id >= ID_PIN && id < ID_PIN + m_aApps.Count())
		{
			OpenApp(m_aApps[id - ID_PIN]);
			return true;
		}

		if (id >= ID_TASK && id < ID_TASK + m_aWindows.Count())
		{
			ROOT_AE3_Window win = m_aWindows[id - ID_TASK];
			if (button == 1)
			{
				TaskMenu(win);
				return true;
			}

			if (win.IsMinimized() || win != m_Focus)
				win.SetMinimized(false);
			else
				win.SetMinimized(true);

			RefreshTaskbar();
			return true;
		}

		ROOT_AE3_Client c = ROOT_AE3_Client.Get();
		switch (id)
		{
			case ID_BG:
			{
				CloseStart();
				SetFocusWindow(null);
				SelectIcon(-1);
				if (button == 1 && LoggedIn())
					BackgroundMenu();

				return true;
			}

			case ID_START: ToggleMenu(); return true;
			case ID_POWER:
			{
				array<string> labels = {"Standby", "Shut down", "-", "Sign out"};
				array<int> ids = {MI_STANDBY, MI_SHUTDOWN, 0, MI_SIGNOUT};
				array<string> icons = {"g_dot", "g_power", "", "g_back"};
				CloseStart();
				ShowMenu(this, labels, ids, icons);
				return true;
			}

			case ID_LOGIN: Login(); return true;
			case ID_TERMINAL: c.Send(ROOT_AE3_EMsg.TERM_KEY, "MODE:0"); return true;
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
