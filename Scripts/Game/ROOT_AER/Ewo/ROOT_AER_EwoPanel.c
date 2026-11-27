//! EWO pack panel of the local player (opened by inventory Use on an EWO pack in the inventory or within reach).
//! Shows the energy pool, the wireless network, the power source and the laptops in the inventory; every change
//! is a request to the server (ROOT_AER_EMsg.EWO), which sends the state (ROOT_AER_EMsg.EWO_STATE) every EWO tick
//! while the panel is open. An empty state closes the panel (pack out of reach, player dead, EWO mode off).
class ROOT_AER_EwoPanel : ChimeraMenuBase
{
	protected static ROOT_AER_EwoPanel s_Open;
	protected static string s_sState;
	protected static RplId s_PackId = RplId.Invalid();

	protected ref ROOT_AER_EwoPanelForm m_Form;

	//------------------------------------------------------------------------------------------------
	static void Open(IEntity pack)
	{
		s_sState = "";
		s_PackId = RplId.Invalid();
		RplComponent rpl;
		if (pack)
			rpl = RplComponent.Cast(pack.FindComponent(RplComponent));

		if (rpl)
			s_PackId = rpl.Id();

		if (!s_Open && !GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ROOT_AER_EwoPanel))
		{
			Print("[AER] EWO panel did not open", LogLevel.WARNING);
			return;
		}

		Send("state");
	}

	//------------------------------------------------------------------------------------------------
	static void Send(string cmd, string a = "", string b = "")
	{
		array<string> f = {cmd, ROOT_AER_Codec.Clean(a), ROOT_AER_Codec.Clean(b)};
		ROOT_AER_Net.SendToServer(s_PackId, ROOT_AER_EMsg.EWO, ROOT_AER_Codec.Join(f));
	}

	//------------------------------------------------------------------------------------------------
	//! Client: state from the server.
	static void OnState(string data)
	{
		if (data.IsEmpty())
		{
			if (s_Open)
				s_Open.Close();

			return;
		}

		s_sState = data;
		if (s_Open && !s_Open.IsEditing())
			s_Open.Rebuild();
	}

	//------------------------------------------------------------------------------------------------
	//! The name or password field has the focus: pushed states wait for the next tick.
	bool IsEditing()
	{
		return m_Form && m_Form.Editing();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		s_Open = this;
		Rebuild();
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, OnBack);
		if (s_Open == this)
		{
			s_Open = null;
			Send("close");
		}

		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack(float value = 0, EActionTrigger reason = 0)
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void Rebuild()
	{
		Widget root = GetRootWidget();
		string ssid, password;
		bool typed;
		if (m_Form)
		{
			typed = m_Form.Typed(ssid, password);
			m_Form.ReleaseHandlers();
		}

		ROOT_AER_UI.Clear(root);
		m_Form = new ROOT_AER_EwoPanelForm(this, root, s_sState);
		if (typed)
			m_Form.Restore(ssid, password);
	}
}

//------------------------------------------------------------------------------------------------
//! Widgets and events of ROOT_AER_EwoPanel.
class ROOT_AER_EwoPanelForm : ROOT_AER_UiListener
{
	protected static const int ID_CLOSE = 1;
	protected static const int ID_WIFI = 2;
	protected static const int ID_SOURCE = 3;
	protected static const int ID_APPLY = 4;
	protected static const int ID_REFRESH = 5;
	protected static const int ID_CHARGE = 100;

	protected ROOT_AER_EwoPanel m_Panel;
	protected EditBoxWidget m_wSsid;
	protected EditBoxWidget m_wPassword;
	protected ref array<string> m_aLaptopIds = {};
	protected ref array<bool> m_aCharging = {};
	protected bool m_bWifi;
	protected bool m_bSource;
	protected string m_sSsid;
	protected string m_sPassword;

	//------------------------------------------------------------------------------------------------
	void ROOT_AER_EwoPanelForm(ROOT_AER_EwoPanel panel, Widget root, string state)
	{
		m_Panel = panel;
		array<string> f = ROOT_AER_Codec.Split(state);
		float sw, sh;
		ROOT_AER_UI.ScreenSize(sw, sh);
		int laptops = 0;
		if (f.Count() > 7)
			laptops = (f.Count() - 7) / 4;

		float w = 620;
		float h = 330 + laptops * 40;
		float x = (sw - w) * 0.5;
		float y = (sh - h) * 0.5;
		Color accent = ROOT_AER_UI.Rgb(0.95, 0.62, 0.12);
		ROOT_AER_UI.Rect(root, 0, 0, sw, sh, ROOT_AER_UI.Rgb(0, 0, 0, 0.45));
		ROOT_AER_UI.Rect(root, x, y, w, h, ROOT_AER_UI.Rgb(0.09, 0.11, 0.10, 0.97));
		ROOT_AER_UI.Rect(root, x, y, w, 4, accent);
		ROOT_AER_UI.Text(root, x + 16, y + 14, w - 120, 26, "77th JSOC EWO pack", 20);
		ROOT_AER_UI.Button(root, x + w - 96, y + 12, 80, 28, "Close", ID_CLOSE, this);
		if (f.Count() < 7)
		{
			ROOT_AER_UI.Text(root, x + 16, y + 60, w - 32, 22, "Waiting for the pack...", 15, ROOT_AER_UI.Rgb(0.75, 0.75, 0.78));
			return;
		}

		float energy = f[0].ToFloat();
		float max = Math.Max(1, f[1].ToFloat());
		m_bWifi = f[2] == "1";
		m_bSource = !f[6].IsEmpty();

		// Energy bar.
		float by = y + 54;
		ROOT_AER_UI.Text(root, x + 16, by, 200, 20, "Energy", 14, ROOT_AER_UI.Rgb(0.75, 0.75, 0.78));
		ROOT_AER_UI.Rect(root, x + 120, by + 2, w - 250, 18, ROOT_AER_UI.Rgb(0.03, 0.03, 0.03));
		float frac = Math.Clamp(energy / max, 0, 1);
		Color bar = ROOT_AER_UI.Rgb(0.25, 0.8, 0.35);
		if (frac < 0.25)
			bar = ROOT_AER_UI.Rgb(0.9, 0.25, 0.2);
		else if (frac < 0.5)
			bar = accent;

		ROOT_AER_UI.Rect(root, x + 122, by + 4, (w - 254) * frac, 14, bar);
		ROOT_AER_UI.Text(root, x + w - 122, by, 110, 20, string.Format("%1 / %2", f[0], f[1]), 14);

		// Network.
		float ny = by + 40;
		string netState = "off";
		if (m_bWifi)
			netState = "broadcasting, gateway " + f[5];

		ROOT_AER_UI.Text(root, x + 16, ny, w - 200, 20, "Network: " + netState, 14);
		string wifiLabel = "Start network";
		if (m_bWifi)
			wifiLabel = "Stop network";

		ROOT_AER_UI.Button(root, x + w - 176, ny - 4, 160, 28, wifiLabel, ID_WIFI, this, accent);
		ROOT_AER_UI.Text(root, x + 16, ny + 38, 100, 20, "Name", 14, ROOT_AER_UI.Rgb(0.75, 0.75, 0.78));
		ROOT_AER_UI.Rect(root, x + 120, ny + 34, 180, 28, ROOT_AER_UI.Rgb(0.04, 0.04, 0.05));
		m_sSsid = f[3];
		m_sPassword = f[4];
		m_wSsid = ROOT_AER_UI.Edit(root, x + 124, ny + 36, 172, 24, f[3], 50, this);
		ROOT_AER_UI.Text(root, x + 312, ny + 38, 80, 20, "Password", 14, ROOT_AER_UI.Rgb(0.75, 0.75, 0.78));
		ROOT_AER_UI.Rect(root, x + 392, ny + 34, 120, 28, ROOT_AER_UI.Rgb(0.04, 0.04, 0.05));
		m_wPassword = ROOT_AER_UI.Edit(root, x + 396, ny + 36, 112, 24, f[4], 51, this);
		ROOT_AER_UI.Button(root, x + w - 96, ny + 34, 80, 28, "Apply", ID_APPLY, this);

		// Power source.
		float py = ny + 80;
		string src = "Power source: none";
		string srcLabel = "Plug in";
		if (m_bSource)
		{
			src = "Power source: " + f[6] + " (recharging)";
			srcLabel = "Unplug";
		}

		ROOT_AER_UI.Text(root, x + 16, py, w - 200, 20, src, 14);
		ROOT_AER_UI.Button(root, x + w - 176, py - 4, 160, 28, srcLabel, ID_SOURCE, this);

		// Laptops.
		float ly = py + 44;
		ROOT_AER_UI.Rect(root, x + 16, ly - 8, w - 32, 1, ROOT_AER_UI.Rgb(0.3, 0.3, 0.3));
		if (laptops == 0)
			ROOT_AER_UI.Text(root, x + 16, ly, w - 32, 20, "No laptops in your inventory.", 14, ROOT_AER_UI.Rgb(0.75, 0.75, 0.78));

		for (int i = 0; i < laptops; i++)
		{
			int k = 7 + i * 4;
			m_aLaptopIds.Insert(f[k]);
			bool charging = f[k + 3] == "1";
			m_aCharging.Insert(charging);
			float ry = ly + i * 40;
			ROOT_AER_UI.Text(root, x + 16, ry + 4, 240, 20, f[k + 1], 14);
			float lvl = Math.Clamp(f[k + 2].ToFloat() / 100, 0, 1);
			ROOT_AER_UI.Rect(root, x + 260, ry + 6, 160, 14, ROOT_AER_UI.Rgb(0.03, 0.03, 0.03));
			ROOT_AER_UI.Rect(root, x + 261, ry + 7, 158 * lvl, 12, ROOT_AER_UI.Rgb(0.3, 0.65, 0.95));
			ROOT_AER_UI.Text(root, x + 428, ry + 4, 50, 20, f[k + 2] + " %", 13);
			string label = "Charge";
			Color bg = null;
			if (charging)
			{
				label = "Stop";
				bg = ROOT_AER_UI.Rgb(0.25, 0.55, 0.3);
			}

			ROOT_AER_UI.Button(root, x + w - 116, ry, 100, 28, label, ID_CHARGE + i, this, bg);
		}

		ROOT_AER_UI.Button(root, x + 16, y + h - 42, 100, 28, "Refresh", ID_REFRESH, this);
		ROOT_AER_UI.Text(root, x + 130, y + h - 38, w - 146, 20, "Charging stops when a laptop is used or leaves the inventory.", 12, ROOT_AER_UI.Rgb(0.6, 0.6, 0.62));
	}

	//------------------------------------------------------------------------------------------------
	bool Editing()
	{
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		return focused && (focused == m_wSsid || focused == m_wPassword);
	}

	//------------------------------------------------------------------------------------------------
	//! Name and password typed but not applied yet.
	bool Typed(out string ssid, out string password)
	{
		if (!m_wSsid || !m_wPassword)
			return false;

		ssid = m_wSsid.GetText();
		password = m_wPassword.GetText();
		return ssid != m_sSsid || password != m_sPassword;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(string ssid, string password)
	{
		if (m_wSsid)
			m_wSsid.SetText(ssid);

		if (m_wPassword)
			m_wPassword.SetText(password);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case ID_CLOSE: m_Panel.Close(); return true;
			case ID_REFRESH: ROOT_AER_EwoPanel.Send("state"); return true;
			case ID_WIFI: ROOT_AER_EwoPanel.Send("wifi", ROOT_AER_Util.BoolStr(!m_bWifi)); return true;
			case ID_SOURCE: ROOT_AER_EwoPanel.Send("source", ROOT_AER_Util.BoolStr(!m_bSource)); return true;
			case ID_APPLY:
			{
				if (m_wSsid && m_wPassword)
					ROOT_AER_EwoPanel.Send("net", m_wSsid.GetText(), m_wPassword.GetText());

				return true;
			}
		}

		int i = id - ID_CHARGE;
		if (i >= 0 && i < m_aLaptopIds.Count())
		{
			ROOT_AER_EwoPanel.Send("charge", m_aLaptopIds[i], ROOT_AER_Util.BoolStr(!m_aCharging[i]));
			return true;
		}

		return false;
	}
}
