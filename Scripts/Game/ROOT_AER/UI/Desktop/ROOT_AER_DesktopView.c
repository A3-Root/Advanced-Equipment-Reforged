//! armaOS desktop: sign-in screen, wallpaper, icons, windows, taskbar, start menu with search, context menus,
//! dialogs, notifications and keyboard shortcuts.
class ROOT_AER_DesktopView : ROOT_AER_GuiReplyHandler
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
	protected static const int ID_SIZE = 19;
	protected static const int ID_SEARCHBAR = 20;
	protected static const int ID_TRAY = 21;
	protected static const int ID_CLOCK = 22;
	protected static const int ID_SHOWDESK = 23;
	protected static const int ID_QUICK_BG = 24;
	protected static const int ID_ICON = 100;
	protected static const int ID_MENU = 300;
	protected static const int ID_POPUP = 400;
	protected static const int ID_TASK = 500;
	protected static const int ID_PIN = 600;
	protected static const int ID_QUICK = 700;
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
	protected static const int MI_SORT = 17;
	protected static const int MI_GROUP = 18;
	protected static const int MI_GRID = 19;
	protected static const int MI_SORT_NAME = 20;
	protected static const int MI_SORT_TYPE = 21;
	protected static const int MI_SORT_SIZE = 22;
	protected static const int MI_GROUP_NONE = 23;
	protected static const int MI_GROUP_TYPE = 24;
	protected static const int ID_FILE = 2000;
	protected static const int ID_SFILE = 3000;
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
	protected FrameWidget m_wQuick;
	protected TextWidget m_wClock;
	protected TextWidget m_wDate;
	protected EditBoxWidget m_wUser;
	protected EditBoxWidget m_wPass;
	protected EditBoxWidget m_wFocusNext;	//!< Field activated next frame (FocusField)
	protected EditBoxWidget m_wSearch;
	protected EditBoxWidget m_wDlgEdit;
	protected TextWidget m_wLoginMsg;
	protected ref array<ref ROOT_AER_Window> m_aWindows = {};
	protected ref array<typename> m_aApps = {};
	protected ref array<int> m_aMenuApps = {};
	protected ref array<int> m_aPopupIds = {};
	protected ROOT_AER_UiListener m_PopupTarget;
	protected ROOT_AER_UiListener m_DialogTarget;
	protected string m_sDialogTag;
	protected ROOT_AER_Window m_Focus;
	protected ROOT_AER_Window m_ContextWindow;
	protected int m_iZ = 10;
	protected int m_iSelIcon = -1;
	protected int m_iSelFile = -1;
	protected static ref map<string, ref ROOT_AER_DeskLayout> s_mLayouts = new map<string, ref ROOT_AER_DeskLayout>();	//!< Computer|user
	protected ref map<string, vector> m_mIconPos = new map<string, vector>();	//!< Icon key -> position drawn
	protected int m_iDragId = -1;	//!< Icon pressed for dragging
	protected bool m_bIconDrag;
	protected float m_fPressX, m_fPressY;
	protected float m_fGrabX, m_fGrabY;	//!< Mouse offset inside the dragged icon
	protected float m_fMenuX, m_fMenuY;	//!< Where the background menu was opened
	protected Widget m_wDragGhost;
	protected ref array<ref array<string>> m_aDeskFiles = {};	//!< ~/Desktop entries (fs_list fields)
	protected ref array<ref array<string>> m_aFound = {};		//!< Start menu file results (fs_find, fs_list fields)
	protected string m_sFindQuery;
	protected ImageWidget m_wWall;
	protected float m_fW, m_fH;
	protected float m_fGridW, m_fGridH;
	protected bool m_bShown;
	protected bool m_bWasTyping;
	protected float m_fClockTimer;
	protected float m_fToastTimer;
	protected ref array<string> m_aTray = {};		//!< ROOT_AER_GuiRouter.Tray fields
	protected string m_sBuiltKey = "-";
	protected ref ROOT_AER_ForwardListener m_IconListener;
	protected ref ROOT_AER_ForwardListener m_TaskbarListener;
	protected ref ROOT_AER_ForwardListener m_MenuListener;
	protected ref ROOT_AER_ForwardListener m_MenuGridListener;
	protected ref ROOT_AER_ForwardListener m_PopupListener;
	protected ref ROOT_AER_ForwardListener m_DialogListener;
	protected ref ROOT_AER_ForwardListener m_ToastListener;
	protected ref ROOT_AER_ForwardListener m_QuickListener;
	protected ref ROOT_AER_FilePicker m_Picker;
	protected RplId m_DeviceId;		//!< Computer of the session the view was built for
	protected static ref array<KeyCode> s_aKeys = {
		KeyCode.KC_RETURN, KeyCode.KC_NUMPADENTER, KeyCode.KC_DELETE, KeyCode.KC_F2, KeyCode.KC_F5, KeyCode.KC_F11, KeyCode.KC_BACK, KeyCode.KC_SPACE,
		KeyCode.KC_UP, KeyCode.KC_DOWN, KeyCode.KC_LEFT, KeyCode.KC_RIGHT, KeyCode.KC_A, KeyCode.KC_C, KeyCode.KC_D, KeyCode.KC_S, KeyCode.KC_X, KeyCode.KC_V, KeyCode.KC_W, KeyCode.KC_TAB,
		KeyCode.KC_0, KeyCode.KC_1, KeyCode.KC_2, KeyCode.KC_3, KeyCode.KC_4, KeyCode.KC_5, KeyCode.KC_6, KeyCode.KC_7, KeyCode.KC_8, KeyCode.KC_9,
		KeyCode.KC_NUMPAD0, KeyCode.KC_NUMPAD1, KeyCode.KC_NUMPAD2, KeyCode.KC_NUMPAD3, KeyCode.KC_NUMPAD4, KeyCode.KC_NUMPAD5, KeyCode.KC_NUMPAD6, KeyCode.KC_NUMPAD7, KeyCode.KC_NUMPAD8, KeyCode.KC_NUMPAD9,
		KeyCode.KC_ADD, KeyCode.KC_SUBTRACT, KeyCode.KC_MULTIPLY, KeyCode.KC_DIVIDE, KeyCode.KC_DECIMAL, KeyCode.KC_PERIOD, KeyCode.KC_COMMA, KeyCode.KC_MINUS, KeyCode.KC_EQUALS, KeyCode.KC_SLASH
	};

	//------------------------------------------------------------------------------------------------
	void ROOT_AER_DesktopView(Widget parent)
	{
		m_wRoot = ROOT_AER_UI.Frame(parent, 0, 0, 1, 1);
		m_DeviceId = ROOT_AER_Client.Get().m_DeviceId;
		ApplySize();
		ROOT_AER_Theme.s_iTheme = ROOT_AER_Client.Get().m_sTheme.ToInt();
		ROOT_AER_Theme.s_iStyle = ROOT_AER_Client.Get().m_iStyle;
		ROOT_AER_AppRegistry.GetApps(m_aApps);
		Build();
	}

	//------------------------------------------------------------------------------------------------
	//! Size and position of the desktop from the client window size (ROOT_AER_Client.DesktopSize).
	protected void ApplySize()
	{
		float sw, sh;
		ROOT_AER_UI.ScreenSize(sw, sh);
		array<float> fractions = {1, 0.85, 0.65, 0.45};
		float frac = fractions[ROOT_AER_Client.Get().DesktopSize()];
		m_fW = sw * frac;
		m_fH = sh * frac;
		ROOT_AER_UI.Place(m_wRoot, (sw - m_fW) / 2, (sh - m_fH) / 2, m_fW, m_fH);
	}

	//------------------------------------------------------------------------------------------------
	//! Next window size (Fullscreen, Large, Medium, Small). Open windows keep their app state and are fitted
	//! into the new area.
	void CycleSize()
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		c.SetDesktopSize((c.DesktopSize() + 1) % 4);
		array<ref ROOT_AER_Window> keep = {};
		foreach (ROOT_AER_Window w : m_aWindows)
		{
			if (!w)
				continue;

			w.Detach();
			keep.Insert(w);
		}

		ROOT_AER_Window focus = m_Focus;
		m_aWindows.Clear();
		ApplySize();
		Build();
		if (LoggedIn())
		{
			foreach (ROOT_AER_Window w : keep)
			{
				m_aWindows.Insert(w);
				w.Relayout();
			}

			if (focus && !focus.IsMinimized())
				focus.BringToFront();

			RefreshTaskbar();
		}
		else
		{
			foreach (ROOT_AER_Window w : keep)
			{
				w.m_App.OnClose();
			}
		}

		array<string> names = {"Fullscreen", "Large", "Medium", "Small"};
		Toast("Window size", names[c.DesktopSize()] + " (F11 or the taskbar button changes it)");
	}

	//------------------------------------------------------------------------------------------------
	//! Open windows of a logged in user are kept by ROOT_AER_Client (Park) for the next session on this computer.
	void Destroy()
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(ApplyFocus);

		ClosePicker();
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		if (LoggedIn() && m_DeviceId != RplId.Invalid() && !m_aWindows.IsEmpty())
		{
			array<ref ROOT_AER_Window> keep = {};
			foreach (ROOT_AER_Window w : m_aWindows)
			{
				if (!w)
					continue;

				w.Detach();
				c.Forget(w.m_App);
				keep.Insert(w);
			}

			c.Park(m_DeviceId, c.m_sUser, keep);
		}
		else
		{
			foreach (ROOT_AER_Window w : m_aWindows)
			{
				w.Destroy();
			}
		}

		ROOT_AER_Theme.s_iStyle = 0;

		m_aWindows.Clear();
		ReleaseHandlers();
		ROOT_AER_Client.Get().Forget(this);
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
		ROOT_AER_Client c = ROOT_AER_Client.Get();
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
		ROOT_AER_UI.Mouse(mx, my);
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
	void SetFocusWindow(ROOT_AER_Window w)
	{
		if (m_Focus == w)
			return;

		m_Focus = w;
		RefreshTaskbar();
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AER_Window GetFocusWindow()
	{
		return m_Focus;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoggedIn()
	{
		return !ROOT_AER_Client.Get().m_sUser.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected bool AppAllowed(ROOT_AER_App app)
	{
		return app && app.IsListed() && (!app.NeedsFileBrowsing() || ROOT_AER_SettingsManager.Get().m_bDesktopEnableFileBrowsing);
	}

	//------------------------------------------------------------------------------------------------
	//! Full rebuild (theme, user or wallpaper changed). Open windows are closed.
	void Build()
	{
		ClosePicker();
		m_sBuiltKey = BuildKey();
		foreach (ROOT_AER_Window w : m_aWindows)
		{
			w.Destroy();
		}

		m_aWindows.Clear();
		m_Focus = null;
		ReleaseHandlers();
		ROOT_AER_UI.Clear(m_wRoot);
		m_wMenu = null;
		m_wPopup = null;
		m_wDialog = null;
		m_wToast = null;
		m_wTaskbar = null;
		m_wQuick = null;
		m_iSelIcon = -1;
		m_iSelFile = -1;
		m_aDeskFiles.Clear();
		ROOT_AER_Theme.s_iTheme = ROOT_AER_Client.Get().m_sTheme.ToInt();
		ROOT_AER_Theme.s_iStyle = ROOT_AER_Client.Get().m_iStyle;
		// Opaque base that takes no input, then the click target for the background.
		ROOT_AER_UI.Rect(m_wRoot, 0, 0, m_fW, m_fH, ROOT_AER_UI.Rgb(0, 0, 0));
		ROOT_AER_UI.Rect(m_wRoot, 0, 0, m_fW, m_fH, ROOT_AER_UI.Rgb(0, 0, 0), ID_BG, this);
		BuildWallpaper(ROOT_AER_Client.Get().m_sWallpaper);
		if (ROOT_AER_Theme.IsTactical())
			BuildGrid();

		if (!LoggedIn())
		{
			BuildLogin();
			return;
		}

		m_wIcons = ROOT_AER_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH - BAR_H);
		BuildIcons();
		RequestDesktopFiles();
		m_wWindows = ROOT_AER_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH - BAR_H, false);
		m_wTaskbar = ROOT_AER_UI.Frame(m_wRoot, 0, m_fH - BAR_H, m_fW, BAR_H);
		m_wTaskbar.SetZOrder(100000);
		RestoreParked();
		RefreshTaskbar();
		PushMirror();
		array<string> none = {};
		ROOT_AER_Client.Get().Request("tray", none, this);
	}

	//------------------------------------------------------------------------------------------------
	//! Windows kept when the last session of this player on this computer ended (same user).
	protected void RestoreParked()
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		array<ref ROOT_AER_Window> parked = {};
		if (!c.TakeParked(m_DeviceId, c.m_sUser, parked))
			return;

		foreach (ROOT_AER_Window w : parked)
		{
			if (!w || !w.m_App)
				continue;

			w.Reattach(this);
			m_aWindows.Insert(w);
			w.Relayout();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Wallpaper pref: 0 black, 1-5 built in, an image file path or a web address.
	protected void BuildWallpaper(string pref)
	{
		m_wWall = null;
		if (pref.StartsWith("/"))
		{
			m_wWall = ROOT_AER_UI.Picture(m_wRoot, 0, 0, m_fW, m_fH, "");
			m_wWall.SetVisible(false);
			array<string> args = {pref};
			ROOT_AER_Client.Get().Request("fs_media", args, this);
			return;
		}

		if (ROOT_AER_WebMedia.KindOf(pref) == "url")
		{
			m_wWall = ROOT_AER_UI.Picture(m_wRoot, 0, 0, m_fW, m_fH, "");
			m_wWall.SetVisible(false);
			ROOT_AER_WebMedia.Request("url", pref, this);
			return;
		}

		ResourceName wall = ROOT_AER_AppRegistry.Wallpaper(pref.ToInt());
		if (!wall.IsEmpty())
			ROOT_AER_UI.Picture(m_wRoot, 0, 0, m_fW, m_fH, wall);
	}

	//------------------------------------------------------------------------------------------------
	//! Tactical style: dark tint over the wallpaper and the node name.
	protected void BuildGrid()
	{
		ROOT_AER_UI.Rect(m_wRoot, 0, 0, m_fW, m_fH, ROOT_AER_UI.Rgb(0, 0, 0, 0.72));
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		string host = c.m_sHostname;
		host.ToUpper();
		ROOT_AER_UI.Text(m_wRoot, m_fW - 420, 10, 400, 18, "NODE " + host, 14, ROOT_AER_Theme.Muted(), true, false, 2);
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
		if (ROOT_AER_Theme.IsTactical())
		{
			BuildLoginTactical();
			return;
		}

		ROOT_AER_Client c = ROOT_AER_Client.Get();
		ROOT_AER_UI.Rect(m_wRoot, 0, 0, m_fW, m_fH, ROOT_AER_UI.Rgb(0, 0, 0, 0.4));
		float w = 320;
		float x = (m_fW - w) * 0.5;
		float y = m_fH * 0.5 - 190;
		Color white = ROOT_AER_UI.Rgb(1, 1, 1);
		ROOT_AER_UI.Icon(m_wRoot, (m_fW - 128) * 0.5, y, 128, "user");
		TextWidget host = ROOT_AER_UI.Text(m_wRoot, 0, y + 142, m_fW, 36, c.m_sHostname, 28, white, false, false, 1);
		host.SetShadow(4, 0xFF000000, 0.5, 0, 1);
		y += 196;
		ROOT_AER_UI.Box(m_wRoot, x, y, w, 36, ROOT_AER_UI.Rgb(1, 1, 1, 0.92), ROOT_AER_UI.Rgb(1, 1, 1, 0.5));
		m_wUser = ROOT_AER_UI.Edit(m_wRoot, x + 10, y + 5, w - 20, 26, "", ID_USER, this);
		ROOT_AER_UI.Placeholder(m_wUser, m_wRoot, x + 10, y + 9, w - 20, "User name", ROOT_AER_UI.Rgb(0.4, 0.4, 0.42));
		y += 46;
		ROOT_AER_UI.Box(m_wRoot, x, y, w, 36, ROOT_AER_UI.Rgb(1, 1, 1, 0.92), ROOT_AER_UI.Rgb(1, 1, 1, 0.5));
		m_wPass = ROOT_AER_UI.Edit(m_wRoot, x + 10, y + 5, w - 20, 26, "", ID_PASS, this, true);
		ROOT_AER_UI.Placeholder(m_wPass, m_wRoot, x + 10, y + 9, w - 20, "Password", ROOT_AER_UI.Rgb(0.4, 0.4, 0.42));
		if (m_wUser)
			m_wUser.SetColor(ROOT_AER_UI.Rgb(0.1, 0.1, 0.1));

		if (m_wPass)
			m_wPass.SetColor(ROOT_AER_UI.Rgb(0.1, 0.1, 0.1));

		y += 52;
		ROOT_AER_UI.Button(m_wRoot, x, y, w, 36, "Sign in", ID_LOGIN, this, ROOT_AER_Theme.Accent(), white, 16);
		y += 48;
		if (c.m_iInterface != 1)
			ROOT_AER_UI.Button(m_wRoot, x + 60, y, w - 120, 30, "Use the terminal", ID_TERMINAL, this, ROOT_AER_UI.Rgb(0, 0, 0, 0), white, 14, ROOT_AER_UI.Rgb(1, 1, 1, 0.15));

		m_wLoginMsg = ROOT_AER_UI.Text(m_wRoot, x - 40, y + 40, w + 80, 44, "", 15, ROOT_AER_UI.Rgb(1, 0.75, 0.7), false, true, 1);
		m_wLoginMsg.SetShadow(4, 0xFF000000, 0.6, 0, 1);
		ROOT_AER_UI.Text(m_wRoot, 0, m_fH - 34, m_fW - 24, 20, "Tab: next field   Enter: sign in", 13, ROOT_AER_UI.Rgb(1, 1, 1, 0.7), false, false, 2);
		if (m_wUser)
			FocusField(m_wUser);
	}

	//------------------------------------------------------------------------------------------------
	//! Tactical sign in: bracketed panel with operator ID and passcode.
	protected void BuildLoginTactical()
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		float w = 460;
		float h = 290;
		if (c.m_iInterface == 1)
			h = 250;

		float x = (m_fW - w) * 0.5;
		float y = Math.Max(8, (m_fH - h) * 0.5);
		Color ink = ROOT_AER_Theme.Accent();
		ROOT_AER_UI.Box(m_wRoot, x, y, w, h, ROOT_AER_Theme.Panel(), ink);
		ROOT_AER_UI.Rect(m_wRoot, x + 1, y + 1, w - 2, 34, ROOT_AER_Theme.Title());
		ROOT_AER_UI.Rect(m_wRoot, x + 1, y + 35, w - 2, 1, ink);
		string host = c.m_sHostname;
		host.ToUpper();
		ROOT_AER_UI.Text(m_wRoot, x + 14, y + 9, w - 28, 18, "[ SECURE TERMINAL ] " + host, 14, ink);
		ROOT_AER_UI.Text(m_wRoot, x + 24, y + 56, 160, 18, "OPERATOR ID", 14, ROOT_AER_Theme.Muted());
		ROOT_AER_UI.Box(m_wRoot, x + 24, y + 76, w - 48, 32, ROOT_AER_Theme.Field(), ROOT_AER_Theme.Border());
		m_wUser = ROOT_AER_UI.Edit(m_wRoot, x + 32, y + 79, w - 64, 26, "", ID_USER, this, false, true);
		ROOT_AER_UI.Text(m_wRoot, x + 24, y + 120, 160, 18, "PASSCODE", 14, ROOT_AER_Theme.Muted());
		ROOT_AER_UI.Box(m_wRoot, x + 24, y + 140, w - 48, 32, ROOT_AER_Theme.Field(), ROOT_AER_Theme.Border());
		m_wPass = ROOT_AER_UI.Edit(m_wRoot, x + 32, y + 143, w - 64, 26, "", ID_PASS, this, true, true);
		if (m_wUser)
			m_wUser.SetColor(ROOT_AER_Theme.Text());

		if (m_wPass)
			m_wPass.SetColor(ROOT_AER_Theme.Text());

		ROOT_AER_UI.Button(m_wRoot, x + 24, y + 190, w - 48, 34, "Authenticate", ID_LOGIN, this, ink, ROOT_AER_Theme.OnAccent(), 15);
		if (c.m_iInterface != 1)
			ROOT_AER_UI.Button(m_wRoot, x + 24, y + 234, w - 48, 32, "Use terminal", ID_TERMINAL, this, ROOT_AER_Theme.Panel(), ROOT_AER_Theme.Text(), 13);

		m_wLoginMsg = ROOT_AER_UI.Text(m_wRoot, x, y + h + 10, w, 40, "", 14, ROOT_AER_Theme.Danger(), false, true, 1);
		ROOT_AER_UI.Text(m_wRoot, 0, m_fH - 30, m_fW - 24, 18, "TAB NEXT FIELD   ENTER AUTHENTICATE", 12, ROOT_AER_Theme.Muted(), false, false, 2);
		if (m_wUser)
			FocusField(m_wUser);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildIcons()
	{
		ROOT_AER_UI.Clear(m_wIcons);
		m_IconListener = new ROOT_AER_ForwardListener(this);
		ROOT_AER_DeskLayout layout = Layout();
		int perCol = IconsPerColumn();
		PlacePending(layout, perCol);
		m_mIconPos.Clear();

		// Icons in automatic order: sort key, then the index into ids.
		array<int> ids = {};
		array<string> order = {};
		foreach (int i, typename t : m_aApps)
		{
			ROOT_AER_App app = ROOT_AER_App.Cast(t.Spawn());
			if (!AppAllowed(app) || !app.IsOnDesktop())
				continue;

			order.Insert(SortKey(layout, 0, app.GetTitle(), "", 0) + "|" + ids.Count().ToString(6));
			ids.Insert(ID_ICON + i);
		}

		foreach (int fi, array<string> e : m_aDeskFiles)
		{
			int rank = 2;
			if (e[1] == "dir")
				rank = 1;

			string name = ROOT_AER_Filesystem.BaseName(e[0]);
			int bytes;
			if (e.Count() > 8)
				bytes = e[8].ToInt();

			order.Insert(SortKey(layout, rank, name, Extension(name), bytes) + "|" + ids.Count().ToString(6));
			ids.Insert(ID_FILE + fi);
		}

		order.Sort();

		// Icons placed by the user keep their position; the others fill free grid cells in order. With grouping each
		// group starts a new column.
		set<int> used = new set<int>();
		foreach (int id : ids)
		{
			string key = IconKey(id);
			if (layout.m_mPos.Contains(key))
				used.Insert(CellAt(layout.m_mPos.Get(key), perCol));
		}

		int next;
		string group;
		foreach (string o : order)
		{
			int id = ids[o.Substring(o.Length() - 6, 6).ToInt()];
			string key = IconKey(id);
			vector pos;
			if (layout.m_mPos.Contains(key))
			{
				pos = layout.m_mPos.Get(key);
			}
			else
			{
				string g = o.Substring(0, 1);
				if (layout.m_iGroup == 1 && !group.IsEmpty() && g != group && next % perCol != 0)
					next = (next / perCol + 1) * perCol;

				group = g;
				while (used.Contains(next))
				{
					next++;
				}

				used.Insert(next);
				pos = CellPos(next, perCol);
			}

			m_mIconPos.Set(key, pos);
			BuildIcon(id, pos[0], pos[1]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Files created from the background menu go where the menu was opened when no icon is there (the shown icons
	//! then keep their places); otherwise they take the first free cell.
	protected void PlacePending(ROOT_AER_DeskLayout layout, int perCol)
	{
		if (layout.m_mPending.IsEmpty())
			return;

		array<string> done = {};
		foreach (string key, vector want : layout.m_mPending)
		{
			bool listed;
			foreach (array<string> e : m_aDeskFiles)
			{
				if ("f:" + e[0] == key)
					listed = true;
			}

			if (!listed)
				continue;

			done.Insert(key);
			vector pos = want;
			if (layout.m_bGrid)
				pos = CellPos(CellAt(pos, perCol), perCol);

			bool free = true;
			foreach (string k, vector p : m_mIconPos)
			{
				if (k == key)
					continue;

				if (layout.m_bGrid && CellAt(p, perCol) == CellAt(pos, perCol))
					free = false;
				else if (!layout.m_bGrid && Math.AbsFloat(p[0] - pos[0]) < CELL_W && Math.AbsFloat(p[1] - pos[1]) < CELL_H)
					free = false;
			}

			if (!free)
				continue;

			foreach (string sk, vector sp : m_mIconPos)
			{
				if (!layout.m_mPos.Contains(sk))
					layout.m_mPos.Set(sk, sp);
			}

			layout.m_mPos.Set(key, pos);
		}

		foreach (string d : done)
		{
			layout.m_mPending.Remove(d);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Icon position under the point where the background menu was opened.
	protected void PendingAt(string path)
	{
		float x = Math.Clamp(m_fMenuX - (CELL_W - 4) * 0.5, 0, m_fW - CELL_W);
		float y = Math.Clamp(m_fMenuY - (CELL_H - 4) * 0.5, 0, m_fH - BAR_H - CELL_H);
		Layout().m_mPending.Set("f:" + path, Vector(x, y, 0));
	}

	//------------------------------------------------------------------------------------------------
	//! Sort key: with grouping or type sort the rank first (0 apps, 1 folders, 2 files).
	protected static string SortKey(ROOT_AER_DeskLayout layout, int rank, string name, string ext, int bytes)
	{
		string lower = name;
		lower.ToLower();
		string head = "0";
		if (layout.m_iGroup == 1 || layout.m_iSort == 1)
			head = rank.ToString();

		switch (layout.m_iSort)
		{
			case 1: return head + ext + "/" + lower;
			case 2: return head + bytes.ToString(12) + "/" + lower;
		}

		return head + lower;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Extension(string name)
	{
		int dot = name.LastIndexOf(".");
		if (dot <= 0)
			return "";

		string ext = name.Substring(dot + 1, name.Length() - dot - 1);
		ext.ToLower();
		return ext;
	}

	//------------------------------------------------------------------------------------------------
	protected static vector CellPos(int cell, int perCol)
	{
		int col = cell / perCol;
		int row = cell % perCol;
		return Vector(8 + col * CELL_W, 8 + row * CELL_H, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Grid cell nearest to an icon position.
	protected static int CellAt(vector pos, int perCol)
	{
		int col = Math.Max(0, Math.Round((pos[0] - 8) / CELL_W));
		int row = Math.Clamp(Math.Round((pos[1] - 8) / CELL_H), 0, perCol - 1);
		return col * perCol + row;
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildIcon(int id, float x, float y)
	{
		string icon, label;
		bool selected;
		if (id >= ID_FILE)
		{
			array<string> e = m_aDeskFiles[id - ID_FILE];
			label = ROOT_AER_Filesystem.BaseName(e[0]);
			icon = ROOT_AER_Icons.ForFile(e[1], e[5] == "1", e[6] == "1", e[7] == "1", label);
			selected = id - ID_FILE == m_iSelFile;
		}
		else
		{
			ROOT_AER_App app = ROOT_AER_App.Cast(m_aApps[id - ID_ICON].Spawn());
			label = IconLabel(app.GetTitle());
			icon = app.GetIcon();
			selected = id - ID_ICON == m_iSelIcon;
		}

		Color bg = ROOT_AER_UI.Rgb(0, 0, 0, 0);
		if (selected)
			bg = ROOT_AER_Theme.Select();

		ROOT_AER_UI.Hit(m_wIcons, x, y, CELL_W - 4, CELL_H - 4, id, m_IconListener, bg, ROOT_AER_UI.Rgb(1, 1, 1, 0.16));
		ROOT_AER_UI.Icon(m_wIcons, x + (CELL_W - 4 - 48) * 0.5, y + 6, 48, icon);
		TextWidget t = ROOT_AER_UI.Text(m_wIcons, x + 2, y + 58, CELL_W - 8, 34, label, 13, IconText(), false, true, 1);
		t.SetShadow(3, 0xFF000000, 0.9, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected int IconsPerColumn()
	{
		return Math.Max(1, Math.Floor((m_fH - BAR_H - 16) / CELL_H));
	}

	//------------------------------------------------------------------------------------------------
	//! Layout key of a desktop icon: app class or file path.
	protected string IconKey(int id)
	{
		if (id >= ID_FILE && m_aDeskFiles.IsIndexValid(id - ID_FILE))
			return "f:" + m_aDeskFiles[id - ID_FILE][0];

		if (id >= ID_ICON && id < ID_ICON + m_aApps.Count())
			return "a:" + m_aApps[id - ID_ICON].ToString();

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Icon positions, sort, grouping and grid of this computer and user, kept for the game session of this client.
	protected ROOT_AER_DeskLayout Layout()
	{
		string owner = m_DeviceId.ToString() + "|" + ROOT_AER_Client.Get().m_sUser;
		ROOT_AER_DeskLayout layout = s_mLayouts.Get(owner);
		if (!layout)
		{
			layout = new ROOT_AER_DeskLayout();
			s_mLayouts.Set(owner, layout);
		}

		return layout;
	}

	//------------------------------------------------------------------------------------------------
	//! Icon image of the dragged icon.
	protected string DragIcon()
	{
		if (m_iDragId >= ID_FILE && m_aDeskFiles.IsIndexValid(m_iDragId - ID_FILE))
		{
			array<string> e = m_aDeskFiles[m_iDragId - ID_FILE];
			return ROOT_AER_Icons.ForFile(e[1], e[5] == "1", e[6] == "1", e[7] == "1", ROOT_AER_Filesystem.BaseName(e[0]));
		}

		if (m_iDragId >= ID_ICON && m_iDragId < ID_ICON + m_aApps.Count())
			return ROOT_AER_App.Cast(m_aApps[m_iDragId - ID_ICON].Spawn()).GetIcon();

		return "file";
	}

	//------------------------------------------------------------------------------------------------
	//! Left button pressed on an icon: dragging starts once the mouse moves.
	protected void PressIcon(int id)
	{
		m_iDragId = id;
		m_bIconDrag = false;
		MouseLocal(m_fPressX, m_fPressY);
		vector pos = m_mIconPos.Get(IconKey(id));
		m_fGrabX = m_fPressX - pos[0];
		m_fGrabY = m_fPressY - pos[1];
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateIconDrag()
	{
		if (m_iDragId < 0)
			return;

		float mx, my;
		MouseLocal(mx, my);
		if (GetGame().GetInputManager().GetActionValue("MouseLeft") <= 0)
		{
			if (m_bIconDrag)
				DropIcon(mx, my);

			EndIconDrag();
			return;
		}

		if (!m_bIconDrag)
		{
			if (Math.AbsFloat(mx - m_fPressX) + Math.AbsFloat(my - m_fPressY) < 8)
				return;

			m_bIconDrag = true;
			m_wDragGhost = ROOT_AER_UI.Frame(m_wRoot, 0, 0, CELL_W, CELL_H, false);
			m_wDragGhost.SetZOrder(99999);
			m_wDragGhost.SetOpacity(0.7);
			ROOT_AER_UI.Rect(m_wDragGhost, 0, 0, CELL_W - 4, CELL_H - 4, ROOT_AER_Theme.Select());
			ROOT_AER_UI.Icon(m_wDragGhost, (CELL_W - 4 - 48) * 0.5, 6, 48, DragIcon());
		}

		FrameSlot.SetPos(m_wDragGhost, mx - m_fGrabX, my - m_fGrabY);
	}

	//------------------------------------------------------------------------------------------------
	//! Puts the dragged icon where it was dropped (snapped to the grid when aligned; an icon already in that cell
	//! takes the old place). Every other icon keeps its position from then on.
	protected void DropIcon(float mx, float my)
	{
		string key = IconKey(m_iDragId);
		if (key.IsEmpty() || !m_mIconPos.Contains(key))
			return;

		ROOT_AER_DeskLayout layout = Layout();
		int perCol = IconsPerColumn();
		float x = Math.Clamp(mx - m_fGrabX, 0, m_fW - CELL_W);
		float y = Math.Clamp(my - m_fGrabY, 0, m_fH - BAR_H - CELL_H);
		vector pos = Vector(x, y, 0);
		vector old = m_mIconPos.Get(key);
		foreach (string k, vector p : m_mIconPos)
		{
			layout.m_mPos.Set(k, p);
		}

		if (layout.m_bGrid)
		{
			int cell = CellAt(pos, perCol);
			pos = CellPos(cell, perCol);
			foreach (string other, vector op : m_mIconPos)
			{
				if (other != key && CellAt(op, perCol) == cell)
					layout.m_mPos.Set(other, old);
			}
		}

		layout.m_mPos.Set(key, pos);
		BuildIcons();
	}

	//------------------------------------------------------------------------------------------------
	protected void EndIconDrag()
	{
		m_iDragId = -1;
		m_bIconDrag = false;
		if (m_wDragGhost)
			m_wDragGhost.RemoveFromHierarchy();

		m_wDragGhost = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Sort: automatic order, user positions dropped.
	protected void SortIcons(int sort)
	{
		ROOT_AER_DeskLayout layout = Layout();
		layout.m_iSort = sort;
		layout.m_mPos.Clear();
		BuildIcons();
	}

	//------------------------------------------------------------------------------------------------
	protected void GroupIcons(int group)
	{
		ROOT_AER_DeskLayout layout = Layout();
		layout.m_iGroup = group;
		layout.m_mPos.Clear();
		BuildIcons();
	}

	//------------------------------------------------------------------------------------------------
	//! Align to grid on: every placed icon moves to the nearest free cell.
	protected void ToggleGrid()
	{
		ROOT_AER_DeskLayout layout = Layout();
		layout.m_bGrid = !layout.m_bGrid;
		if (layout.m_bGrid)
		{
			int perCol = IconsPerColumn();
			set<int> used = new set<int>();
			map<string, vector> snapped = new map<string, vector>();
			foreach (string k, vector p : layout.m_mPos)
			{
				int cell = CellAt(p, perCol);
				while (used.Contains(cell))
				{
					cell++;
				}

				used.Insert(cell);
				snapped.Set(k, CellPos(cell, perCol));
			}

			layout.m_mPos.Clear();
			foreach (string sk, vector sp : snapped)
			{
				layout.m_mPos.Set(sk, sp);
			}
		}

		BuildIcons();
	}

	//------------------------------------------------------------------------------------------------
	protected void SortMenu()
	{
		array<string> labels = {"Name", "Type", "Size"};
		labels[Layout().m_iSort] = labels[Layout().m_iSort] + "  (current)";
		array<int> ids = {MI_SORT_NAME, MI_SORT_TYPE, MI_SORT_SIZE};
		ShowMenu(this, labels, ids);
	}

	//------------------------------------------------------------------------------------------------
	protected void GroupMenu()
	{
		array<string> labels = {"None", "Type (apps, folders, files)"};
		labels[Layout().m_iGroup] = labels[Layout().m_iGroup] + "  (current)";
		array<int> ids = {MI_GROUP_NONE, MI_GROUP_TYPE};
		ShowMenu(this, labels, ids);
	}

	//------------------------------------------------------------------------------------------------
	protected static Color IconText()
	{
		if (ROOT_AER_Theme.IsTactical())
			return ROOT_AER_Theme.Text();

		return ROOT_AER_UI.Rgb(1, 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected static string IconLabel(string title)
	{
		if (!ROOT_AER_Theme.IsTactical())
			return title;

		string t = title;
		t.ToUpper();
		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! Desktop files come from ~/Desktop (file browsing setting on).
	protected bool DesktopFiles()
	{
		return LoggedIn() && ROOT_AER_SettingsManager.Get().m_bDesktopEnableFileBrowsing;
	}

	//------------------------------------------------------------------------------------------------
	protected string DesktopDir()
	{
		return ROOT_AER_Filesystem.HomeOf(ROOT_AER_Client.Get().m_sUser) + "/Desktop";
	}

	//------------------------------------------------------------------------------------------------
	void RequestDesktopFiles()
	{
		if (!DesktopFiles())
			return;

		array<string> args = {DesktopDir()};
		ROOT_AER_Client.Get().Request("fs_list", args, this);
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

		OpenEntry(m_aDeskFiles[index]);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens an fs_list entry with the app for its kind (folder, image, locked file, program, text).
	protected void OpenEntry(notnull array<string> e)
	{
		if (e.Count() < 8)
			return;

		if (e[1] == "dir")
		{
			OpenApp(ROOT_AER_AppFiles, e[0]);
			return;
		}

		if (e[5] == "1")
		{
			OpenApp(ROOT_AER_AppMedia, e[0]);
			return;
		}

		if (e[6] == "1")
		{
			OpenApp(ROOT_AER_AppFiles, ROOT_AER_Filesystem.Parent(e[0]));
			return;
		}

		if (e[7] == "1")
		{
			array<string> args = {e[0]};
			ROOT_AER_Client.Get().Request("run_file", args, this);
			return;
		}

		OpenApp(ROOT_AER_AppNotepad, e[0]);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (tag == ROOT_AER_WebMedia.TAG)
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

		if (!ROOT_AER_Filesystem.ValidName(value))
		{
			Toast("armaOS", "That name is not allowed.");
			return;
		}

		ROOT_AER_Client c = ROOT_AER_Client.Get();
		string path = DesktopDir() + "/" + value;
		switch (tag)
		{
			case "desk_newfolder":
			{
				PendingAt(path);
				array<string> a = {path};
				c.Request("fs_mkdir", a, this);
				return;
			}

			case "desk_newfile":
			{
				PendingAt(path);
				array<string> b = {path, ""};
				c.Request("fs_save", b, this);
				return;
			}

			case "desk_rename":
			{
				if (!m_aDeskFiles.IsIndexValid(m_iSelFile))
					return;

				ROOT_AER_DeskLayout renamed = Layout();
				string oldKey = "f:" + m_aDeskFiles[m_iSelFile][0];
				if (renamed.m_mPos.Contains(oldKey))
					renamed.m_mPos.Set("f:" + path, renamed.m_mPos.Get(oldKey));
				else if (m_mIconPos.Contains(oldKey))
					renamed.m_mPending.Set("f:" + path, m_mIconPos.Get(oldKey));

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
		array<typename> want = {ROOT_AER_AppFiles, ROOT_AER_AppBrowser, ROOT_AER_AppMail, ROOT_AER_AppTerminal};
		foreach (typename t : want)
		{
			if (m_aApps.Contains(t) && AppAllowed(ROOT_AER_App.Cast(t.Spawn())))
				pinned.Insert(t);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AER_Window FirstWindowOf(typename t)
	{
		foreach (ROOT_AER_Window w : m_aWindows)
		{
			if (w.m_App.Type() == t)
				return w;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Tray state from the server ("tray"): external power, battery percent (-1 none), address, network name,
	//! wired, USB drives.
	protected string TrayField(int i)
	{
		if (i < m_aTray.Count())
			return m_aTray[i];

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected int TrayInt(int i)
	{
		string v = TrayField(i);
		if (v.IsEmpty())
			return 0;

		return v.ToInt();
	}

	//------------------------------------------------------------------------------------------------
	protected bool TrayOnline()
	{
		return !TrayField(2).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected int TrayBattery()
	{
		if (TrayField(1).IsEmpty())
			return -1;

		return TrayInt(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void TaskButton(float x, ROOT_AER_Window w, string icon, int id)
	{
		Color bg = ROOT_AER_UI.Rgb(0, 0, 0, 0);
		if (w && w == m_Focus && !w.IsMinimized())
			bg = ROOT_AER_Theme.Select();

		ROOT_AER_UI.IconButton(m_wTaskbar, x, 4, 44, BAR_H - 8, icon, id, m_TaskbarListener, 24, null, bg, ROOT_AER_Theme.Hover());
		if (!w)
			return;

		float iw = 6;
		Color ic = ROOT_AER_Theme.Muted();
		if (w == m_Focus && !w.IsMinimized())
		{
			iw = 18;
			ic = ROOT_AER_Theme.Accent();
		}

		ROOT_AER_UI.Rect(m_wTaskbar, x + (44 - iw) * 0.5, BAR_H - 7, iw, 3, ic);
	}

	//------------------------------------------------------------------------------------------------
	//! Windows taskbar: start, search, pinned apps and open windows (left); window size, tray (network, power,
	//! USB: quick panel), clock (calendar) and show desktop (right).
	void RefreshTaskbar()
	{
		if (!m_wTaskbar)
			return;

		ROOT_AER_UI.Clear(m_wTaskbar);
		m_TaskbarListener = new ROOT_AER_ForwardListener(this);
		m_wClock = null;
		m_wDate = null;
		if (ROOT_AER_Theme.IsTactical())
		{
			RefreshTaskbarTactical();
			return;
		}

		Color clear = ROOT_AER_UI.Rgb(0, 0, 0, 0);
		Color hover = ROOT_AER_Theme.Hover();
		ROOT_AER_UI.Rect(m_wTaskbar, 0, 0, m_fW, BAR_H, ROOT_AER_Theme.Taskbar());
		ROOT_AER_UI.Rect(m_wTaskbar, 0, 0, m_fW, 1, ROOT_AER_Theme.Border());
		Color startBg = clear;
		if (m_wMenu)
			startBg = ROOT_AER_Theme.Select();

		ROOT_AER_UI.IconButton(m_wTaskbar, 6, 4, 44, BAR_H - 8, "logo", ID_START, m_TaskbarListener, 26, null, startBg, hover);
		float x = 56;
		if (m_fW >= 960)
		{
			ROOT_AER_UI.Hit(m_wTaskbar, x, 9, 210, BAR_H - 18, ID_SEARCHBAR, m_TaskbarListener, ROOT_AER_Theme.Field(), hover);
			ROOT_AER_UI.Icon(m_wTaskbar, x + 12, 16, 16, "g_search", ROOT_AER_Theme.Muted());
			ROOT_AER_UI.Text(m_wTaskbar, x + 36, 15, 166, 18, "Search apps", 13, ROOT_AER_Theme.Muted());
			x += 218;
		}
		else
		{
			ROOT_AER_UI.IconButton(m_wTaskbar, x, 4, 44, BAR_H - 8, "g_search", ID_SEARCHBAR, m_TaskbarListener, 18, null, clear, hover);
			x += 48;
		}

		ROOT_AER_UI.Rect(m_wTaskbar, x, 12, 1, BAR_H - 24, ROOT_AER_Theme.Border());
		x += 8;

		// Right side, from the edge inwards.
		ROOT_AER_UI.Hit(m_wTaskbar, m_fW - 8, 0, 8, BAR_H, ID_SHOWDESK, m_TaskbarListener, clear, hover);
		ROOT_AER_UI.Rect(m_wTaskbar, m_fW - 9, 12, 1, BAR_H - 24, ROOT_AER_Theme.Border());
		float cx = m_fW - 104;
		Color clockBg = clear;
		ROOT_AER_UI.Hit(m_wTaskbar, cx, 4, 92, BAR_H - 8, ID_CLOCK, m_TaskbarListener, clockBg, hover);
		m_wClock = ROOT_AER_UI.Text(m_wTaskbar, cx, 7, 92, 18, "", 14, ROOT_AER_Theme.Text(), false, false, 1);
		m_wDate = ROOT_AER_UI.Text(m_wTaskbar, cx, 25, 92, 16, "", 12, ROOT_AER_Theme.Muted(), false, false, 1);

		int bat = TrayBattery();
		bool usb = TrayInt(5) > 0;
		float tw = 16 + 28 + 28;
		if (bat >= 0)
			tw += 34;

		if (usb)
			tw += 28;

		float tx = cx - 4 - tw;
		Color trayBg = clear;
		if (m_wQuick)
			trayBg = ROOT_AER_Theme.Select();

		ROOT_AER_UI.Hit(m_wTaskbar, tx, 4, tw, BAR_H - 8, ID_TRAY, m_TaskbarListener, trayBg, hover);
		float ix = tx + 8;
		if (usb)
		{
			ROOT_AER_UI.Icon(m_wTaskbar, ix + 4, 15, 18, "drive");
			ix += 28;
		}

		string netIcon = "g_wifi";
		if (TrayField(4) == "1")
			netIcon = "network";

		Color netTint = ROOT_AER_Theme.Text();
		if (!TrayOnline())
			netTint = ROOT_AER_Theme.Muted();

		ROOT_AER_UI.Icon(m_wTaskbar, ix + 4, 15, 18, netIcon, netTint);
		if (!TrayOnline())
			ROOT_AER_UI.Rect(m_wTaskbar, ix + 16, 28, 6, 6, ROOT_AER_Theme.Danger());

		ix += 28;
		string powIcon = "g_battery";
		if (TrayField(0) == "1")
			powIcon = "g_plug";

		Color powTint = ROOT_AER_Theme.Text();
		if (bat >= 0 && bat <= 15 && TrayField(0) != "1")
			powTint = ROOT_AER_Theme.Danger();

		ROOT_AER_UI.Icon(m_wTaskbar, ix + 4, 15, 18, powIcon, powTint);
		ix += 28;
		if (bat >= 0)
			ROOT_AER_UI.Text(m_wTaskbar, ix - 4, 15, 40, 18, bat.ToString() + "%", 12, ROOT_AER_Theme.Text());

		ROOT_AER_UI.IconButton(m_wTaskbar, tx - 40, 4, 36, BAR_H - 8, "g_max", ID_SIZE, m_TaskbarListener, 16, null, clear, hover);
		float limit = tx - 48;

		array<typename> pinned = {};
		GetPinned(pinned);
		array<ROOT_AER_Window> shown = {};
		foreach (typename t : pinned)
		{
			if (x + 44 > limit)
				break;

			ROOT_AER_Window w = FirstWindowOf(t);
			if (w)
			{
				shown.Insert(w);
				TaskButton(x, w, w.m_App.GetIcon(), ID_TASK + m_aWindows.Find(w));
			}
			else
			{
				ROOT_AER_App app = ROOT_AER_App.Cast(t.Spawn());
				TaskButton(x, null, app.GetIcon(), ID_PIN + m_aApps.Find(t));
			}

			x += 48;
		}

		foreach (int i, ROOT_AER_Window w2 : m_aWindows)
		{
			if (shown.Contains(w2))
				continue;

			if (x + 44 > limit)
				break;

			TaskButton(x, w2, w2.m_App.GetIcon(), ID_TASK + i);
			x += 48;
		}

		UpdateClock();
	}

	//------------------------------------------------------------------------------------------------
	//! Tactical taskbar: menu and search, text tabs of pinned apps and windows, status segments (network,
	//! power, USB: quick panel), size, clock (calendar), show desktop.
	protected void RefreshTaskbarTactical()
	{
		Color ink = ROOT_AER_Theme.Accent();
		Color clear = ROOT_AER_UI.Rgb(0, 0, 0, 0);
		ROOT_AER_UI.Rect(m_wTaskbar, 0, 0, m_fW, BAR_H, ROOT_AER_Theme.Taskbar());
		ROOT_AER_UI.Rect(m_wTaskbar, 0, 0, m_fW, 1, ink);
		Color startBg = ROOT_AER_Theme.Panel();
		Color startFg = ink;
		if (m_wMenu)
		{
			startBg = ink;
			startFg = ROOT_AER_Theme.OnAccent();
		}

		ROOT_AER_UI.Button(m_wTaskbar, 6, 8, 76, BAR_H - 16, "Menu", ID_START, m_TaskbarListener, startBg, startFg, 14);
		ROOT_AER_UI.Button(m_wTaskbar, 88, 8, 44, BAR_H - 16, "/", ID_SEARCHBAR, m_TaskbarListener, ROOT_AER_Theme.Panel(), ink, 14);
		float x = 140;

		// Right side, from the edge inwards.
		ROOT_AER_UI.Hit(m_wTaskbar, m_fW - 8, 0, 8, BAR_H, ID_SHOWDESK, m_TaskbarListener, clear, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.Rect(m_wTaskbar, m_fW - 9, 10, 1, BAR_H - 20, ink);
		float cx = m_fW - 100;
		ROOT_AER_UI.Hit(m_wTaskbar, cx, 4, 88, BAR_H - 8, ID_CLOCK, m_TaskbarListener, clear, ROOT_AER_Theme.Hover());
		m_wClock = ROOT_AER_UI.Text(m_wTaskbar, cx, 7, 88, 18, "", 14, ink, true, false, 1);
		m_wDate = ROOT_AER_UI.Text(m_wTaskbar, cx, 25, 88, 16, "", 12, ROOT_AER_Theme.Muted(), true, false, 1);

		string net = "NET OFF";
		if (TrayOnline())
			net = "NET " + TrayField(2);

		string pow = "PWR";
		if (TrayField(0) == "1")
			pow += " EXT";

		if (TrayBattery() >= 0)
			pow += " " + TrayField(1) + "%";

		string status = net + "  " + pow;
		if (TrayInt(5) > 0)
			status += "  USB " + TrayField(5);

		float sw = 24 + status.Length() * 8;
		float sx = cx - 6 - sw;
		Color statusBg = clear;
		if (m_wQuick)
			statusBg = ROOT_AER_Theme.Select();

		ROOT_AER_UI.Hit(m_wTaskbar, sx, 6, sw, BAR_H - 12, ID_TRAY, m_TaskbarListener, statusBg, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.Text(m_wTaskbar, sx, 15, sw, 18, status, 13, ROOT_AER_Theme.Text(), true, false, 1);
		ROOT_AER_UI.Button(m_wTaskbar, sx - 58, 10, 52, 28, "Size", ID_SIZE, m_TaskbarListener, ROOT_AER_Theme.Panel(), ROOT_AER_Theme.Text(), 12);
		float limit = sx - 64;

		array<typename> pinned = {};
		GetPinned(pinned);
		array<ROOT_AER_Window> shown = {};
		foreach (typename t : pinned)
		{
			ROOT_AER_Window w = FirstWindowOf(t);
			string title;
			int id;
			if (w)
			{
				shown.Insert(w);
				title = w.m_App.GetTitle();
				id = ID_TASK + m_aWindows.Find(w);
			}
			else
			{
				ROOT_AER_App app = ROOT_AER_App.Cast(t.Spawn());
				title = app.GetTitle();
				id = ID_PIN + m_aApps.Find(t);
			}

			if (x + TabWidth(title) > limit)
				break;

			x += TaskTab(x, w, title, id);
		}

		foreach (int i, ROOT_AER_Window w2 : m_aWindows)
		{
			if (shown.Contains(w2))
				continue;

			if (x + TabWidth(w2.m_App.GetTitle()) > limit)
				break;

			x += TaskTab(x, w2, w2.m_App.GetTitle(), ID_TASK + i);
		}

		UpdateClock();
	}

	//------------------------------------------------------------------------------------------------
	protected static string TabLabel(string title)
	{
		string label = title;
		label.ToUpper();
		if (label.Length() > 10)
			label = label.Substring(0, 10);

		return label;
	}

	//------------------------------------------------------------------------------------------------
	protected static float TabWidth(string title)
	{
		return 30 + TabLabel(title).Length() * 9;
	}

	//------------------------------------------------------------------------------------------------
	//! Tactical taskbar tab: upper case title, underline for open windows; returns the width used.
	protected float TaskTab(float x, ROOT_AER_Window w, string title, int id)
	{
		string label = TabLabel(title);
		float tw = TabWidth(title) - 4;
		Color fg = ROOT_AER_Theme.Muted();
		if (w)
			fg = ROOT_AER_Theme.Text();

		Color bg = ROOT_AER_UI.Rgb(0, 0, 0, 0);
		if (w && w == m_Focus && !w.IsMinimized())
			bg = ROOT_AER_Theme.Select();

		ROOT_AER_UI.Hit(m_wTaskbar, x, 6, tw, BAR_H - 12, id, m_TaskbarListener, bg, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.Text(m_wTaskbar, x, 15, tw, 18, label, 13, fg, true, false, 1);
		if (w)
			ROOT_AER_UI.Rect(m_wTaskbar, x + 4, BAR_H - 9, tw - 8, 2, ROOT_AER_Theme.Accent());

		return tw + 4;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateClock()
	{
		string now = ROOT_AER_Util.Now();
		if (m_wClock && m_wDate && now.Length() >= 16)
		{
			m_wClock.SetText(now.Substring(11, 5));
			m_wDate.SetText(now.Substring(0, 10));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Minimizes every window; when none is open, restores them.
	protected void ShowDesktop()
	{
		bool any;
		foreach (ROOT_AER_Window w : m_aWindows)
		{
			if (!w.IsMinimized())
				any = true;
		}

		foreach (ROOT_AER_Window w2 : m_aWindows)
		{
			w2.SetMinimized(any);
		}

		if (any)
			SetFocusWindow(null);

		RefreshTaskbar();
	}

	// Quick panel -----------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Panel above the tray: user, network, power and USB state, shortcuts and power buttons.
	protected void ToggleQuick()
	{
		if (m_wQuick)
		{
			CloseQuick();
			return;
		}

		CloseStart();
		CloseMenu();
		bool tac = ROOT_AER_Theme.IsTactical();
		float w = Math.Min(360, m_fW - 16);
		float h = Math.Min(372, m_fH - BAR_H - 16);
		float x = m_fW - w - 8;
		float y = m_fH - BAR_H - h - 8;
		m_QuickListener = new ROOT_AER_ForwardListener(this);
		m_wQuick = ROOT_AER_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH - BAR_H, false);
		m_wQuick.SetZOrder(100001);
		ROOT_AER_UI.Rect(m_wQuick, 0, 0, m_fW, m_fH - BAR_H, ROOT_AER_UI.Rgb(0, 0, 0, 0), ID_QUICK_BG, m_QuickListener);
		ROOT_AER_UI.Shadow(m_wQuick, x, y, w, h, 12);
		Color edge;
		if (tac)
			edge = ROOT_AER_Theme.Accent();

		ROOT_AER_UI.Box(m_wQuick, x, y, w, h, ROOT_AER_Theme.Panel(), edge);
		ROOT_AER_UI.Rect(m_wQuick, x + 1, y + 1, w - 2, h - 2, ROOT_AER_UI.Rgb(0, 0, 0, 0), ID_QUICK + 99, m_QuickListener);

		ROOT_AER_Client c = ROOT_AER_Client.Get();
		ROOT_AER_UI.Icon(m_wQuick, x + 16, y + 14, 32, "user");
		ROOT_AER_UI.Text(m_wQuick, x + 58, y + 14, w - 70, 18, c.m_sUser, 15, ROOT_AER_Theme.Text(), tac);
		ROOT_AER_UI.Text(m_wQuick, x + 58, y + 33, w - 70, 16, c.m_sHostname, 12, ROOT_AER_Theme.Muted(), tac);
		ROOT_AER_UI.Rect(m_wQuick, x + 12, y + 58, w - 24, 1, ROOT_AER_Theme.Border());

		string net = "Not connected";
		string netIcon = "g_wifi";
		if (TrayField(4) == "1")
			netIcon = "network";

		if (TrayOnline())
		{
			string via = "Wi-Fi";
			if (TrayField(4) == "1")
				via = "Cable";

			net = via;
			if (!TrayField(3).IsEmpty())
				net += " - " + TrayField(3);

			net += "  " + TrayField(2);
		}

		int bat = TrayBattery();
		string pow = "No power information";
		if (TrayField(0) == "1")
			pow = "External power";
		else if (bat >= 0)
			pow = "On battery";

		if (bat >= 0)
			pow += " - battery " + bat.ToString() + "%";

		string usb = "No flash drive";
		int drives = TrayInt(5);
		if (drives == 1)
			usb = "1 flash drive";
		else if (drives > 1)
			usb = drives.ToString() + " flash drives";

		float ry = y + 70;
		QuickRow(x + 16, ry, w - 32, netIcon, "Network", net);
		ry += 40;
		QuickRow(x + 16, ry, w - 32, "g_battery", "Power", pow);
		if (bat >= 0)
		{
			Color fill = ROOT_AER_Theme.Accent();
			if (bat <= 15)
				fill = ROOT_AER_Theme.Danger();

			ROOT_AER_UI.Rect(m_wQuick, x + 44, ry + 34, w - 60, 4, ROOT_AER_Theme.Field());
			ROOT_AER_UI.Rect(m_wQuick, x + 44, ry + 34, (w - 60) * bat / 100.0, 4, fill);
		}

		ry += 44;
		QuickRow(x + 16, ry, w - 32, "drive", "Storage", usb);
		ROOT_AER_UI.Button(m_wQuick, x + w - 112, ry + 4, 96, 28, "Flash drives", ID_QUICK + 13, m_QuickListener, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Text(), 12, ROOT_AER_Theme.Hover());
		ry += 44;

		array<string> labels = {"Network", "My Computer", "Terminal", "Theme: " + ROOT_AER_Theme.Name(c.m_sTheme.ToInt()), "Window size", "Settings"};
		array<string> icons = {"network", "computer", "terminal", "photos", "g_max", "settings"};
		float tw = (w - 32 - 16) / 3;
		float th = 54;
		foreach (int i, string label : labels)
		{
			int col = i % 3;
			int row = i / 3;
			float tx = x + 16 + col * (tw + 8);
			float ty = ry + row * (th + 8);
			ROOT_AER_UI.Hit(m_wQuick, tx, ty, tw, th, ID_QUICK + i, m_QuickListener, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Hover());
			ROOT_AER_UI.Icon(m_wQuick, tx + (tw - 18) * 0.5, ty + 8, 18, icons[i]);
			ROOT_AER_UI.Text(m_wQuick, tx + 2, ty + 32, tw - 4, 16, label, 11, ROOT_AER_Theme.Text(), tac, false, 1);
		}

		float by = y + h - 46;
		ROOT_AER_UI.Rect(m_wQuick, x + 1, by - 8, w - 2, 1, ROOT_AER_Theme.Border());
		float bw = (w - 32 - 16) / 3;
		ROOT_AER_UI.Button(m_wQuick, x + 16, by, bw, 32, "Standby", ID_QUICK + 10, m_QuickListener, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Text(), 13, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.Button(m_wQuick, x + 24 + bw, by, bw, 32, "Shut down", ID_QUICK + 11, m_QuickListener, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Text(), 13, ROOT_AER_Theme.Hover());
		ROOT_AER_UI.Button(m_wQuick, x + 32 + bw * 2, by, bw, 32, "Sign out", ID_QUICK + 12, m_QuickListener, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Text(), 13, ROOT_AER_Theme.Hover());
		RefreshTaskbar();
	}

	//------------------------------------------------------------------------------------------------
	protected void QuickRow(float x, float y, float w, string icon, string label, string value)
	{
		bool tac = ROOT_AER_Theme.IsTactical();
		ROOT_AER_UI.Icon(m_wQuick, x, y + 6, 20, icon);
		ROOT_AER_UI.Text(m_wQuick, x + 28, y, w - 28, 16, label, 12, ROOT_AER_Theme.Muted(), tac);
		ROOT_AER_UI.Text(m_wQuick, x + 28, y + 16, w - 28, 18, value, 13, ROOT_AER_Theme.Text(), tac);
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseQuick()
	{
		if (!m_wQuick)
			return;

		m_wQuick.RemoveFromHierarchy();
		m_wQuick = null;
		RefreshTaskbar();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnQuick(int index)
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		array<string> none = {};
		if (index == 3)
		{
			string next = ((c.m_sTheme.ToInt() + 1) % 3).ToString();
			array<string> args = {"theme", next};
			c.Request("sys_set", args, this);
			CloseQuick();
			ApplyPrefs(next, c.m_sWallpaper);
			return;
		}

		CloseQuick();
		switch (index)
		{
			case 0: OpenApp(ROOT_AER_AppNetwork); return;
			case 1: OpenApp(ROOT_AER_AppMyComputer); return;
			case 2: c.Send(ROOT_AER_EMsg.TERM_KEY, "MODE:0"); return;
			case 4: CycleSize(); return;
			case 5: OpenApp(ROOT_AER_AppSettings); return;
			case 10: c.Request("standby", none, this); return;
			case 11: c.Request("shutdown", none, this); return;
			case 12: c.Request("signout", none, this); return;
			case 13: OpenApp(ROOT_AER_AppMyComputer); return;
		}
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

		if (m_wQuick)
		{
			m_wQuick.RemoveFromHierarchy();
			m_wQuick = null;
		}

		float w = Math.Min(620, m_fW - 12);
		float h = Math.Min(560, m_fH - BAR_H - 12);
		float x = 6;
		float y = m_fH - BAR_H - h - 6;
		m_MenuListener = new ROOT_AER_ForwardListener(this);
		m_wMenu = ROOT_AER_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH - BAR_H, false);
		m_wMenu.SetZOrder(100001);
		ROOT_AER_UI.Rect(m_wMenu, 0, 0, m_fW, m_fH - BAR_H, ROOT_AER_UI.Rgb(0, 0, 0, 0), ID_MENU_BG, m_MenuListener);
		ROOT_AER_UI.Shadow(m_wMenu, x, y, w, h, 14);
		ROOT_AER_UI.Box(m_wMenu, x, y, w, h, ROOT_AER_Theme.Panel());
		// Swallows clicks on the panel background.
		ROOT_AER_UI.Rect(m_wMenu, x + 1, y + 1, w - 2, h - 2, ROOT_AER_UI.Rgb(0, 0, 0, 0), ID_START + 1000, m_MenuListener);
		ROOT_AER_UI.Box(m_wMenu, x + 24, y + 22, w - 48, 36, ROOT_AER_Theme.Field());
		ROOT_AER_UI.Icon(m_wMenu, x + 36, y + 32, 16, "g_search", ROOT_AER_Theme.Muted());
		m_wSearch = ROOT_AER_UI.Edit(m_wMenu, x + 62, y + 27, w - 100, 26, "", ID_SEARCH, m_MenuListener);
		if (m_wSearch)
			m_wSearch.SetColor(ROOT_AER_Theme.Text());

		string hint = "Search apps and files";
		string apps = "Apps";
		if (ROOT_AER_Theme.IsTactical())
		{
			hint = "SEARCH";
			apps = "[ PROGRAMS ]";
		}

		ROOT_AER_UI.Placeholder(m_wSearch, m_wMenu, x + 62, y + 31, w - 100, hint, ROOT_AER_Theme.Muted());
		ROOT_AER_UI.Text(m_wMenu, x + 32, y + 72, 300, 20, apps, 15, ROOT_AER_Theme.Text());
		m_fGridW = w - 40;
		m_fGridH = h - 100 - 72;
		m_wMenuGrid = ROOT_AER_UI.Frame(m_wMenu, x + 20, y + 100, m_fGridW, m_fGridH);
		ROOT_AER_UI.Rect(m_wMenu, x + 1, y + h - 64, w - 2, 63, ROOT_AER_Theme.Bar());
		ROOT_AER_UI.Rect(m_wMenu, x + 1, y + h - 65, w - 2, 1, ROOT_AER_Theme.Border());
		ROOT_AER_Client c = ROOT_AER_Client.Get();
		ROOT_AER_UI.Icon(m_wMenu, x + 28, y + h - 48, 32, "user");
		ROOT_AER_UI.Text(m_wMenu, x + 70, y + h - 50, 300, 18, c.m_sUser, 15, ROOT_AER_Theme.Text());
		ROOT_AER_UI.Text(m_wMenu, x + 70, y + h - 30, 300, 16, c.m_sHostname, 12, ROOT_AER_Theme.Muted());
		ROOT_AER_UI.IconButton(m_wMenu, x + w - 64, y + h - 56, 44, 44, "g_power", ID_POWER, m_MenuListener, 20);
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

		ROOT_AER_UI.Clear(m_wMenuGrid);
		m_MenuGridListener = new ROOT_AER_ForwardListener(this);
		m_aMenuApps.Clear();
		string f = filter.Trim();
		f.ToLower();
		float gw = m_fGridW;
		float gh = m_fGridH;
		if (ROOT_AER_Theme.IsTactical())
		{
			FillMenuList(f);
			return;
		}

		float filesH = FoundHeight(f);
		int cols = Math.Max(1, Math.Floor(gw / 96));
		float cw = gw / cols;
		int maxRows = Math.Max(1, Math.Floor((gh - filesH) / 86));
		foreach (int i, typename t : m_aApps)
		{
			ROOT_AER_App app = ROOT_AER_App.Cast(t.Spawn());
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
			ROOT_AER_UI.Hit(m_wMenuGrid, x + 2, y, cw - 4, 82, ID_MENU + i, m_MenuGridListener, ROOT_AER_UI.Rgb(0, 0, 0, 0));
			ROOT_AER_UI.Icon(m_wMenuGrid, x + (cw - 36) * 0.5, y + 10, 36, app.GetIcon());
			ROOT_AER_UI.Text(m_wMenuGrid, x + 4, y + 52, cw - 8, 30, title, 12, ROOT_AER_Theme.Text(), false, true, 1);
		}

		float fy = Math.Ceil(m_aMenuApps.Count() / (cols * 1.0)) * 86 + 4;
		if (m_aMenuApps.IsEmpty() && m_aFound.IsEmpty())
		{
			string none = "No apps match your search.";
			if (f.Length() >= 2)
				none = "No apps or files match your search.";

			ROOT_AER_UI.Text(m_wMenuGrid, 0, 40, gw, 20, none, 14, ROOT_AER_Theme.Muted(), false, false, 1);
		}

		BuildFound(f, fy, gw);
	}

	//------------------------------------------------------------------------------------------------
	protected float FoundHeight(string f)
	{
		if (f.Length() < 2 || m_aFound.IsEmpty())
			return 0;

		return 30 + Math.Min(m_aFound.Count(), 6) * 30;
	}

	//------------------------------------------------------------------------------------------------
	//! Start menu file results: heading, then rows with icon, name and folder.
	protected void BuildFound(string f, float y, float gw)
	{
		if (f.Length() < 2 || m_aFound.IsEmpty())
			return;

		bool tac = ROOT_AER_Theme.IsTactical();
		string heading = "Files";
		if (tac)
			heading = "[ FILES ]";

		ROOT_AER_UI.Text(m_wMenuGrid, 12, y, gw - 24, 20, heading, 14, ROOT_AER_Theme.Muted(), tac);
		y += 26;
		int shown;
		foreach (int i, array<string> e : m_aFound)
		{
			if (shown >= 6 || y + 28 > m_fGridH)
				break;

			shown++;
			string name = ROOT_AER_Filesystem.BaseName(e[0]);
			ROOT_AER_UI.Hit(m_wMenuGrid, 4, y, gw - 8, 28, ID_SFILE + i, m_MenuGridListener, ROOT_AER_UI.Rgb(0, 0, 0, 0));
			ROOT_AER_UI.Icon(m_wMenuGrid, 12, y + 5, 18, ROOT_AER_Icons.ForFile(e[1], e[5] == "1", e[6] == "1", e[7] == "1", name));
			ROOT_AER_UI.Text(m_wMenuGrid, 40, y + 5, gw * 0.45, 18, name, 13, ROOT_AER_Theme.Text(), tac);
			ROOT_AER_UI.Text(m_wMenuGrid, 40 + gw * 0.45, y + 5, gw * 0.55 - 52, 18, ROOT_AER_Filesystem.Parent(e[0]), 12, ROOT_AER_Theme.Muted(), tac, false, 2);
			y += 30;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Tactical start menu: two columns of numbered program rows.
	protected void FillMenuList(string f)
	{
		float gw = m_fGridW;
		float gh = m_fGridH - FoundHeight(f);
		int rows = Math.Max(1, Math.Floor(gh / 30));
		float cw = gw * 0.5;
		foreach (int i, typename t : m_aApps)
		{
			ROOT_AER_App app = ROOT_AER_App.Cast(t.Spawn());
			if (!AppAllowed(app))
				continue;

			string title = app.GetTitle();
			string low = title;
			low.ToLower();
			if (!f.IsEmpty() && !low.Contains(f))
				continue;

			int k = m_aMenuApps.Count();
			if (k >= rows * 2)
				break;

			m_aMenuApps.Insert(i);
			int col = k / rows;
			int row = k % rows;
			float x = col * cw;
			float y = row * 30;
			title.ToUpper();
			string num = (k + 1).ToString();
			if (k + 1 < 10)
				num = "0" + num;

			ROOT_AER_UI.Hit(m_wMenuGrid, x + 2, y, cw - 8, 28, ID_MENU + i, m_MenuGridListener, ROOT_AER_UI.Rgb(0, 0, 0, 0));
			ROOT_AER_UI.Text(m_wMenuGrid, x + 10, y + 6, 40, 18, num, 13, ROOT_AER_Theme.Muted(), true);
			ROOT_AER_UI.Icon(m_wMenuGrid, x + 44, y + 5, 18, app.GetIcon());
			ROOT_AER_UI.Text(m_wMenuGrid, x + 72, y + 6, cw - 84, 18, title, 13, ROOT_AER_Theme.Text(), true);
		}

		if (m_aMenuApps.IsEmpty() && m_aFound.IsEmpty())
			ROOT_AER_UI.Text(m_wMenuGrid, 0, 40, gw, 20, "NO MATCH", 14, ROOT_AER_Theme.Muted(), true, false, 1);

		int used = Math.Min(m_aMenuApps.Count(), rows);
		BuildFound(f, used * 30 + 4, gw);
	}

	// Context menu ----------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Popup menu at the cursor (x / y below 0) or at x / y. Label "-" is a separator.
	//! The chosen id goes to target.OnMenuItem.
	void ShowMenu(ROOT_AER_UiListener target, notnull array<string> labels, notnull array<int> ids, array<string> icons = null, float x = -1, float y = -1)
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
		m_PopupListener = new ROOT_AER_ForwardListener(this);
		m_wPopup = ROOT_AER_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH, false);
		m_wPopup.SetZOrder(200000);
		ROOT_AER_UI.Rect(m_wPopup, 0, 0, m_fW, m_fH, ROOT_AER_UI.Rgb(0, 0, 0, 0), ID_POPUP_BG, m_PopupListener);
		ROOT_AER_UI.Shadow(m_wPopup, px, py, w, h, 8);
		ROOT_AER_UI.Box(m_wPopup, px, py, w, h, ROOT_AER_Theme.Panel());
		float ry = py + 4;
		foreach (int k, string label : labels)
		{
			if (label == "-")
			{
				ROOT_AER_UI.Rect(m_wPopup, px + 8, ry + 4, w - 16, 1, ROOT_AER_Theme.Border());
				ry += 9;
				continue;
			}

			string icon = "";
			if (icons && k < icons.Count())
				icon = icons[k];

			ROOT_AER_UI.ItemButton(m_wPopup, px + 4, ry, w - 8, 32, icon, label, ID_POPUP + k, m_PopupListener, 14);
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
	void ShowDialog(ROOT_AER_UiListener target, string tag, string title, string message, bool input = false, string value = "", string okLabel = "OK", bool password = false)
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
		m_DialogListener = new ROOT_AER_ForwardListener(this);
		m_wDialog = ROOT_AER_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH, false);
		m_wDialog.SetZOrder(150000);
		ROOT_AER_UI.Rect(m_wDialog, 0, 0, m_fW, m_fH, ROOT_AER_UI.Rgb(0, 0, 0, 0.25), ID_DLG_BG, m_DialogListener);
		ROOT_AER_UI.Shadow(m_wDialog, x, y, w, h, 16);
		ROOT_AER_UI.Box(m_wDialog, x, y, w, h, ROOT_AER_Theme.Panel());
		ROOT_AER_UI.Text(m_wDialog, x + 24, y + 20, w - 48, 24, title, 18, ROOT_AER_Theme.Text());
		ROOT_AER_UI.Text(m_wDialog, x + 24, y + 54, w - 48, 44, message, 14, ROOT_AER_Theme.Text(), false, true);
		m_wDlgEdit = null;
		if (input)
		{
			ROOT_AER_UI.Box(m_wDialog, x + 24, y + 104, w - 48, 32, ROOT_AER_Theme.Field(), ROOT_AER_Theme.Accent());
			m_wDlgEdit = ROOT_AER_UI.Edit(m_wDialog, x + 32, y + 107, w - 64, 26, value, ID_DLG_EDIT, m_DialogListener, password);
			if (m_wDlgEdit)
				m_wDlgEdit.SetColor(ROOT_AER_Theme.Text());
		}

		ROOT_AER_UI.Rect(m_wDialog, x + 1, y + h - 64, w - 2, 63, ROOT_AER_Theme.Bar());
		ROOT_AER_UI.Button(m_wDialog, x + w - 252, y + h - 48, 110, 32, okLabel, ID_DLG_OK, m_DialogListener, ROOT_AER_Theme.Accent(), ROOT_AER_Theme.OnAccent(), 14);
		ROOT_AER_UI.Rect(m_wDialog, x + w - 134, y + h - 48, 110, 32, ROOT_AER_Theme.Border());
		ROOT_AER_UI.Button(m_wDialog, x + w - 133, y + h - 47, 108, 30, "Cancel", ID_DLG_CANCEL, m_DialogListener, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Text(), 14);
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

		ROOT_AER_UiListener target = m_DialogTarget;
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
	void ShowFilePicker(ROOT_AER_UiListener target, string tag, string title, string start = "", bool save = false, string fileName = "", string okLabel = "Open", bool folders = false)
	{
		ClosePicker();
		CloseMenu();
		CloseDialog();
		float aw, ah;
		GetArea(aw, ah);
		m_Picker = new ROOT_AER_FilePicker(this, m_wRoot, aw, ah, target, tag, title, start, save, fileName, okLabel, folders);
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
	ROOT_AER_App OpenApp(typename t, string arg = "")
	{
		CloseStart();
		if (!LoggedIn())
			return null;

		foreach (ROOT_AER_Window w : m_aWindows)
		{
			if (w.m_App.Type() == t && arg.IsEmpty())
			{
				w.SetMinimized(false);
				RefreshTaskbar();
				return w.m_App;
			}
		}

		ROOT_AER_App app = ROOT_AER_App.Cast(t.Spawn());
		if (!app)
			return null;

		app.m_sArg = arg;
		if (app.Launch())
			return null;

		int slot = m_aWindows.Count() % 8;
		float offset = slot * 30;
		vector size = app.GetSize();
		float aw, ah;
		GetArea(aw, ah);
		float x = Math.Max(0, Math.Min((aw - size[0]) * 0.5 - 60 + offset, aw - size[0]));
		float y = Math.Max(0, Math.Min((ah - size[1]) * 0.35 + offset, ah - size[1]));
		ROOT_AER_Window win = new ROOT_AER_Window(this, app, x, y);
		m_aWindows.Insert(win);
		win.BringToFront();
		RefreshTaskbar();
		PushMirror();
		return app;
	}

	//------------------------------------------------------------------------------------------------
	void CloseWindow(ROOT_AER_Window w)
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

		if (m_wQuick)
		{
			CloseQuick();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Sends the window list for the device screen mirror.
	protected void PushMirror()
	{
		string s = "armaOS desktop - " + ROOT_AER_Client.Get().m_sUser;
		foreach (ROOT_AER_Window w : m_aWindows)
		{
			s += "\n[ " + w.m_App.GetTitle() + " ]";
		}

		array<string> args = {s};
		ROOT_AER_Client.Get().Request("ui_mirror", args, null);
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt)
	{
		ROOT_AER_UI.UpdatePlaceholders();
		UpdateKeys();
		UpdateIconDrag();
		foreach (ROOT_AER_Window w : m_aWindows)
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
				ROOT_AER_Client.Get().Request("tray", args, this);
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
		bool typing = ROOT_AER_UI.Typing();
		bool wasTyping = m_bWasTyping;
		m_bWasTyping = typing;
		bool ctrl = Debug.KeyState(KeyCode.KC_LCONTROL) != 0 || Debug.KeyState(KeyCode.KC_RCONTROL) != 0;
		foreach (KeyCode key : s_aKeys)
		{
			if (!ROOT_AER_Keys.Pressed(key))
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

		FocusField(next);
	}

	//------------------------------------------------------------------------------------------------
	//! Ends write mode of every field, then activates edit next frame: activating it inside the change event of
	//! another edit box leaves both in write mode.
	protected void FocusField(EditBoxWidget edit)
	{
		m_wFocusNext = edit;
		GetGame().GetWorkspace().SetFocusedWidget(null);
		GetGame().GetCallqueue().Remove(ApplyFocus);
		GetGame().GetCallqueue().CallLater(ApplyFocus, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyFocus()
	{
		EditBoxWidget e = m_wFocusNext;
		m_wFocusNext = null;
		if (!e || (e != m_wUser && e != m_wPass && e != m_wDlgEdit))
			return;

		GetGame().GetWorkspace().SetFocusedWidget(e);
		e.ActivateWriteMode();
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleKey(KeyCode key, bool ctrl)
	{
		if (key == KeyCode.KC_F11)
		{
			CycleSize();
			return;
		}

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
			ROOT_AER_App app = ROOT_AER_App.Cast(t.Spawn());
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
		m_ToastListener = new ROOT_AER_ForwardListener(this);
		m_wToast = ROOT_AER_UI.Frame(m_wRoot, 0, 0, m_fW, m_fH, false);
		m_wToast.SetZOrder(100002);
		ROOT_AER_UI.Shadow(m_wToast, x, y, w, h, 10);
		ROOT_AER_UI.Box(m_wToast, x, y, w, h, ROOT_AER_Theme.Panel());
		ROOT_AER_UI.Rect(m_wToast, x + 1, y + 1, w - 2, h - 2, ROOT_AER_UI.Rgb(0, 0, 0, 0), ID_TOAST, m_ToastListener);
		ROOT_AER_UI.Icon(m_wToast, x + 16, y + 16, 28, "logo");
		ROOT_AER_UI.Text(m_wToast, x + 58, y + 14, w - 72, 20, title, 15, ROOT_AER_Theme.Text());
		ROOT_AER_UI.Text(m_wToast, x + 58, y + 38, w - 72, 46, text, 13, ROOT_AER_Theme.Muted(), false, true);
		m_fToastTimer = 6;
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
		array<string> f = ROOT_AER_Codec.Split(data);
		if (f.Count() < 3)
			return;

		foreach (ROOT_AER_Window w : m_aWindows)
		{
			ROOT_AER_AppGames g = ROOT_AER_AppGames.Cast(w.m_App);
			if (g)
				g.OnState(f[0], f[1].ToInt(), f[2]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Re-reads theme and wallpaper and rebuilds.
	void ApplyPrefs(string theme, string wallpaper)
	{
		ROOT_AER_Client c = ROOT_AER_Client.Get();
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
		ROOT_AER_Client.Get().Request("login", args, this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (cmd == "login")
		{
			if (!ok && m_wLoginMsg)
			{
				m_wLoginMsg.SetText(ROOT_AER_App.Arg(payload, 0));
				if (m_wPass)
				{
					ROOT_AER_UI.SetEdit(m_wPass, "");
					FocusField(m_wPass);
				}
			}

			return;
		}

		if (cmd == "tray" && ok)
		{
			string before = ROOT_AER_Codec.Join(m_aTray);
			m_aTray.Clear();
			m_aTray.Copy(payload);
			if (ROOT_AER_Codec.Join(m_aTray) != before)
				RefreshTaskbar();

			return;
		}

		if (cmd == "fs_media")
		{
			if (!ok)
			{
				Toast("armaOS", "Background: " + ROOT_AER_App.Arg(payload, 0));
				return;
			}

			string kind = ROOT_AER_App.Arg(payload, 3);
			if (ROOT_AER_WebMedia.IsWeb(kind))
				ROOT_AER_WebMedia.Request(kind, ROOT_AER_App.Arg(payload, 1), this);
			else
				ShowWallpaper(ROOT_AER_App.Arg(payload, 1), false);

			return;
		}

		if (cmd == "fs_find")
		{
			if (!ok || payload.IsEmpty() || payload[0] != m_sFindQuery || !m_wMenu)
				return;

			m_aFound.Clear();
			for (int fi = 1; fi < payload.Count(); fi++)
			{
				array<string> fe = ROOT_AER_Codec.SplitSub(payload[fi]);
				if (fe.Count() >= 8)
					m_aFound.Insert(fe);
			}

			FillMenuGrid(m_sFindQuery);
			return;
		}

		if (cmd == "fs_list")
		{
			if (!ok || payload.IsEmpty() || payload[0] != DesktopDir())
				return;

			m_aDeskFiles.Clear();
			for (int i = 1; i < payload.Count(); i++)
			{
				array<string> e = ROOT_AER_Codec.SplitSub(payload[i]);
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
				string q = m_wSearch.GetText().Trim();
				m_aFound.Clear();
				m_sFindQuery = q;
				FillMenuGrid(q);
				if (q.Length() >= 2)
				{
					string root = ROOT_AER_Filesystem.HomeOf(ROOT_AER_Client.Get().m_sUser);
					if (ROOT_AER_SettingsManager.Get().m_bDesktopEnableFileBrowsing)
						root = "/";

					array<string> args = {root, q};
					ROOT_AER_Client.Get().Request("fs_find", args, this);
				}

				return true;
			}

			if (!m_aMenuApps.IsEmpty())
			{
				OpenApp(m_aApps[m_aMenuApps[0]]);
			}
			else if (!m_aFound.IsEmpty())
			{
				array<string> first = m_aFound[0];
				CloseStart();
				OpenEntry(first);
			}

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

		// Write mode also ends on a click elsewhere or Tab: only Enter moves on or signs in.
		bool enter = Debug.KeyState(KeyCode.KC_RETURN) != 0 || Debug.KeyState(KeyCode.KC_NUMPADENTER) != 0;
		if (id == ID_USER && m_wPass)
		{
			if (enter)
				FocusField(m_wPass);

			return true;
		}

		if (id == ID_PASS && !enter)
			return true;

		if (id == ID_PASS)
		{
			Login();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseDown(Widget w, int id, int button)
	{
		if (button == 0 && ((id >= ID_ICON && id < ID_ICON + m_aApps.Count()) || (id >= ID_FILE && id < ID_FILE + m_aDeskFiles.Count())))
		{
			PressIcon(id);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseUp(Widget w, int id, int button)
	{
		foreach (ROOT_AER_Window win : m_aWindows)
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
		MouseLocal(m_fMenuX, m_fMenuY);
		string grid = "Align icons to grid: Off";
		if (Layout().m_bGrid)
			grid = "Align icons to grid: On";

		array<string> labels = {"Sort by...", "Group by...", grid, "Refresh", "-", "Open Files", "Open Terminal", "-", "Personalize", "System info"};
		array<int> ids = {MI_SORT, MI_GROUP, MI_GRID, MI_REFRESH, 0, MI_FILES, MI_TERMINAL, 0, MI_PERSONALIZE, MI_SYSINFO};
		array<string> icons = {"g_chevron_down", "g_list", "g_grid", "g_refresh", "", "folder", "terminal", "", "settings", "info"};
		if (DesktopFiles())
		{
			labels.InsertAt("-", 4);
			ids.InsertAt(0, 4);
			icons.InsertAt("", 4);
			labels.InsertAt("New text document", 4);
			ids.InsertAt(MI_NEW_FILE, 4);
			icons.InsertAt("g_newfile", 4);
			labels.InsertAt("New folder", 4);
			ids.InsertAt(MI_NEW_FOLDER, 4);
			icons.InsertAt("g_newfolder", 4);
		}

		ShowMenu(this, labels, ids, icons);
	}

	//------------------------------------------------------------------------------------------------
	protected void TaskMenu(ROOT_AER_Window win)
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
					string old = ROOT_AER_Filesystem.BaseName(m_aDeskFiles[m_iSelFile][0]);
					ShowDialog(this, "desk_rename", "Rename", "New name for \"" + old + "\":", true, old, "Rename");
				}

				return;
			}

			case MI_FILE_DELETE:
			{
				if (m_aDeskFiles.IsIndexValid(m_iSelFile))
				{
					array<string> del = {m_aDeskFiles[m_iSelFile][0]};
					ROOT_AER_Client.Get().Request("fs_delete", del, this);
				}

				return;
			}
			case MI_SORT: SortMenu(); return;
			case MI_GROUP: GroupMenu(); return;
			case MI_GRID: ToggleGrid(); return;
			case MI_SORT_NAME: SortIcons(0); return;
			case MI_SORT_TYPE: SortIcons(1); return;
			case MI_SORT_SIZE: SortIcons(2); return;
			case MI_GROUP_NONE: GroupIcons(0); return;
			case MI_GROUP_TYPE: GroupIcons(1); return;
			case MI_PERSONALIZE: OpenApp(ROOT_AER_AppSettings); return;
			case MI_SYSINFO: OpenApp(ROOT_AER_AppSysInfo); return;
			case MI_TERMINAL: OpenApp(ROOT_AER_AppTerminal); return;
			case MI_FILES: OpenApp(ROOT_AER_AppFiles); return;
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
		ROOT_AER_Client c = ROOT_AER_Client.Get();
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
			ROOT_AER_UiListener target = m_PopupTarget;
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

		if (id == ID_START + 1000 || id == ID_QUICK + 99)
			return true;

		if (id == ID_QUICK_BG)
		{
			CloseQuick();
			return true;
		}

		if (id >= ID_QUICK && id <= ID_QUICK + 13)
		{
			OnQuick(id - ID_QUICK);
			return true;
		}

		if (id >= ID_SFILE && id < ID_SFILE + m_aFound.Count())
		{
			array<string> found = m_aFound[id - ID_SFILE];
			CloseStart();
			OpenEntry(found);
			return true;
		}

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
			ROOT_AER_Window win = m_aWindows[id - ID_TASK];
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

		ROOT_AER_Client c = ROOT_AER_Client.Get();
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
			case ID_SEARCHBAR:
			{
				if (!m_wMenu)
					ToggleMenu();

				return true;
			}

			case ID_TRAY: ToggleQuick(); return true;
			case ID_CLOCK: OpenApp(ROOT_AER_AppCalendar); return true;
			case ID_SHOWDESK: ShowDesktop(); return true;
			case ID_SIZE: CycleSize(); return true;
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
			case ID_TERMINAL: c.Send(ROOT_AER_EMsg.TERM_KEY, "MODE:0"); return true;
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

//! Desktop icon layout of one computer and user (client).
class ROOT_AER_DeskLayout
{
	ref map<string, vector> m_mPos = new map<string, vector>();	//!< Icon key -> position placed by the user
	ref map<string, vector> m_mPending = new map<string, vector>();	//!< New or renamed file -> wanted position
	int m_iSort;			//!< 0 name, 1 type, 2 size
	int m_iGroup;			//!< 0 none, 1 type
	bool m_bGrid = true;	//!< Align icons to grid
}
