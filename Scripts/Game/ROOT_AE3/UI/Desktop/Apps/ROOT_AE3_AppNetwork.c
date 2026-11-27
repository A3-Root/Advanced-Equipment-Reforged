//! Wi-Fi: connection status, networks in range, connect (password asked for secured networks), disconnect.
//! Remote access (SSH logins) on / off for root and sudoers. Address and router settings are set by the Game Master or the mission.
class ROOT_AE3_AppNetwork : ROOT_AE3_App
{
	protected static const int ID_CONNECT = 1;
	protected static const int ID_DISCONNECT = 2;
	protected static const int ID_REFRESH = 3;
	protected static const int ID_SSH = 4;

	protected ROOT_AE3_ListView m_List;
	protected ref array<string> m_aGateways = {};
	protected ref array<string> m_aSsids = {};
	protected ref array<bool> m_aSecured = {};
	protected ref array<string> m_aInfo = {};
	protected string m_sPending;
	protected float m_fTimer;

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Network";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "network";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(620, 500, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Connected()
	{
		return !m_aInfo.IsEmpty() && m_aInfo[0] != "not connected";
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();
		ROOT_AE3_UI.Box(GetContent(), 12, 12, w - 24, 76, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.Icon(GetContent(), 26, 26, 48, "network");
		string title = "Not connected";
		string sub = "Choose a network below and press Connect.";
		if (m_aInfo.IsEmpty())
		{
			title = "Checking...";
			sub = "";
		}
		else if (Connected())
		{
			title = "Connected to " + Arg(m_aInfo, 2);
			sub = string.Format("Address %1   Gateway %2", Arg(m_aInfo, 0), Arg(m_aInfo, 1));
		}

		Label(88, 26, w - 220, 24, title, 18);
		ROOT_AE3_UI.Text(GetContent(), 88, 52, w - 220, 20, sub, 13, ROOT_AE3_Theme.Muted());
		if (Connected())
			Btn(w - 130, 34, 106, 30, "Disconnect", ID_DISCONNECT);

		if (!m_aInfo.IsEmpty())
		{
			string ssh = "Remote access: off";
			if (Arg(m_aInfo, 3) == "on")
				ssh = "Remote access: on";

			Btn(w - 290, 100, 172, 28, ssh, ID_SSH);
		}

		Label(12, 104, 300, 22, "Networks in range", 15);
		Btn(w - 110, 100, 98, 28, "Refresh", ID_REFRESH);
		m_List = List(1, 12, 134, w - 24, h - 186);
		array<string> headers = {"Network", "Signal", "Security"};
		array<float> widths = {(w - 24) * 0.5, (w - 24) * 0.25, (w - 24) * 0.25};
		m_List.SetColumns(headers, widths);
		m_List.m_sEmpty = "No networks in range.";
		m_List.Render();
		MainBtn(w - 132, h - 42, 120, 30, "Connect", ID_CONNECT);
		ROOT_AE3_UI.Text(GetContent(), 12, h - 36, w - 160, 20, "Double click a network to connect.", 13, ROOT_AE3_Theme.Muted());
		if (m_aInfo.IsEmpty())
			Request0("net_info");

		Request0("net_scan");
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		m_fTimer += dt;
		if (m_fTimer > 10)
		{
			m_fTimer = 0;
			Request0("net_scan");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Signal bars from the distance text "<n> m".
	protected static string Signal(string distance)
	{
		float d = distance.ToFloat();
		if (d < 30)
			return "Excellent";

		if (d < 80)
			return "Good";

		if (d < 150)
			return "Fair";

		return "Weak";
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			if (cmd == "net_connect" && !m_sPending.IsEmpty())
			{
				int i = m_aGateways.Find(m_sPending);
				if (i >= 0 && m_aSecured[i])
					AskPassword(i, Arg(payload, 0));
			}

			return;
		}

		switch (cmd)
		{
			case "net_info":
			{
				m_aInfo.Clear();
				m_aInfo.Copy(payload);
				if (m_aInfo.IsEmpty())
					m_aInfo.Insert("not connected");

				RedrawKeep();

				return;
			}

			case "net_scan":
			{
				int sel = -1;
				string selGw;
				if (m_List && m_List.m_iSelected >= 0 && m_List.m_iSelected < m_aGateways.Count())
					selGw = m_aGateways[m_List.m_iSelected];

				m_List.Clear();
				m_aGateways.Clear();
				m_aSsids.Clear();
				m_aSecured.Clear();
				string current = Arg(m_aInfo, 2);
				foreach (string p : payload)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(p);
					if (e.Count() < 4)
						continue;

					bool secured = e[3] != "open";
					string name = e[0];
					if (Connected() && name == current)
						name += "  (connected)";

					string sec = "Open";
					string icon = "g_wifi";
					if (secured)
					{
						sec = "Secured";
						icon = "file_locked";
					}

					array<string> row = {name, Signal(e[2]), sec};
					m_List.AddRow(row, icon);
					if (e[1] == selGw)
						sel = m_aGateways.Count();

					m_aGateways.Insert(e[1]);
					m_aSsids.Insert(e[0]);
					m_aSecured.Insert(secured);
				}

				m_List.m_iSelected = sel;
				m_List.Render();
				return;
			}

			case "net_ssh":
			case "net_connect":
			case "net_disconnect":
			{
				m_sPending = "";
				Status(Arg(payload, 0));
				Request0("net_info");
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Rebuild without dropping the list content.
	protected void RedrawKeep()
	{
		int sel = -1;
		if (m_List)
			sel = m_List.m_iSelected;

		m_aLists.Clear();
		ReleaseHandlers();
		ROOT_AE3_UI.Clear(GetContent());
		Build();
		m_List.m_iSelected = sel;
	}

	//------------------------------------------------------------------------------------------------
	protected void AskPassword(int i, string message)
	{
		if (!m_Desktop)
			return;

		m_sPending = m_aGateways[i];
		m_Desktop.ShowDialog(this, "wifi", "Connect to " + m_aSsids[i], message, true, "", "Connect", true);
	}

	//------------------------------------------------------------------------------------------------
	protected void ConnectSelected()
	{
		int i = m_List.m_iSelected;
		if (i < 0 || i >= m_aGateways.Count())
		{
			Status("Select a network first.");
			return;
		}

		if (m_aSecured[i])
		{
			AskPassword(i, "This network is secured. Enter its password.");
			return;
		}

		m_sPending = m_aGateways[i];
		Request2("net_connect", m_aGateways[i], "");
		Status("Connecting to " + m_aSsids[i] + "...");
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (tag != "wifi" || !ok || m_sPending.IsEmpty())
		{
			m_sPending = "";
			return;
		}

		Request2("net_connect", m_sPending, value);
		Status("Connecting...");
	}

	//------------------------------------------------------------------------------------------------
	override void OnListActivate(int listId, int row)
	{
		ConnectSelected();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnKey(KeyCode key, bool ctrl)
	{
		if (key == KeyCode.KC_RETURN || key == KeyCode.KC_NUMPADENTER)
		{
			ConnectSelected();
			return true;
		}

		if (key == KeyCode.KC_F5)
		{
			Request0("net_scan");
			return true;
		}

		if (key == KeyCode.KC_UP || key == KeyCode.KC_DOWN)
		{
			int d = 1;
			if (key == KeyCode.KC_UP)
				d = -1;

			m_List.Move(d);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case ID_CONNECT: ConnectSelected(); return true;
			case ID_DISCONNECT: Request0("net_disconnect"); return true;
			case ID_REFRESH: Request0("net_scan"); return true;
			case ID_SSH:
			{
				string on = "1";
				if (Arg(m_aInfo, 3) == "on")
					on = "0";

				Request1("net_ssh", on);
				return true;
			}
		}

		return false;
	}
}
