//! Network: status, scan, connect, disconnect, static IP, SSH access, router administration.
class ROOT_AE3_AppNetwork : ROOT_AE3_App
{
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
		return Vector(760, 520, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_ListView m_List;
	protected ref array<string> m_aGateways = {};
	protected TextWidget m_wInfo;
	protected EditBoxWidget m_wPassword;
	protected EditBoxWidget m_wIp;
	protected EditBoxWidget m_wRouterPw;
	protected TextWidget m_wRouter;
	protected EditBoxWidget m_wSsid;
	protected EditBoxWidget m_wNewPw;
	protected EditBoxWidget m_wRange;

	override void Build()
	{
		float w = Width();
		float h = Height();
		m_wInfo = Label(8, 6, w - 16, 44, "", 14, true, true);
		m_List = List(1, 6, 54, w - 12, 150);
		array<string> headers = {"Network", "Gateway", "Distance", "Security"};
		array<float> widths = {(w - 12) * 0.35, (w - 12) * 0.25, (w - 12) * 0.2, (w - 12) * 0.2};
		m_List.SetColumns(headers, widths);
		float y = 210;
		Btn(6, y, 80, 26, "Scan", 1);
		Label(94, y + 4, 70, 22, "Password");
		m_wPassword = Field(164, y, 160, "", 2, true);
		Btn(330, y, 90, 26, "Connect", 3);
		Btn(424, y, 100, 26, "Disconnect", 4);
		y += 34;
		Label(8, y + 4, 80, 22, "Static IP");
		m_wIp = Field(90, y, 160, "", 5);
		Btn(256, y, 80, 26, "Set IP", 6);
		Btn(342, y, 120, 26, "SSH on/off", 7);
		y += 40;
		Label(8, y, w - 16, 22, "Router administration (connected router)", 16);
		y += 26;
		Label(8, y + 4, 120, 22, "Admin password");
		m_wRouterPw = Field(130, y, 160, "", 8, true);
		Btn(296, y, 80, 26, "Load", 9);
		y += 32;
		m_wRouter = Label(8, y, w - 16, 44, "", 13, true, true);
		y += 48;
		Label(8, y + 4, 40, 22, "SSID");
		m_wSsid = Field(50, y, 150, "", 10);
		Label(206, y + 4, 70, 22, "Password");
		m_wNewPw = Field(276, y, 140, "", 11);
		Label(422, y + 4, 50, 22, "Range");
		m_wRange = Field(472, y, 70, "", 12);
		Btn(548, y, 80, 26, "Save", 13);
		Btn(632, y, w - 638, 26, "Ext. SSH", 14);
		Request0("net_info");
		Request0("net_scan");
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		switch (cmd)
		{
			case "net_info":
			{
				m_wInfo.SetText(Arg(payload, 0));
				return;
			}

			case "net_scan":
			{
				m_List.Clear();
				m_aGateways.Clear();
				foreach (string p : payload)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(p);
					if (e.Count() < 4)
						continue;

					m_aGateways.Insert(e[1]);
					m_List.AddRow(e);
				}

				m_List.Render();
				Status(string.Format("%1 networks in range", m_aGateways.Count()));
				return;
			}

			case "router_page":
			{
				m_wRouter.SetText(Arg(payload, 0));
				ROOT_AE3_UI.SetEdit(m_wSsid, Arg(payload, 1));
				ROOT_AE3_UI.SetEdit(m_wNewPw, Arg(payload, 2));
				ROOT_AE3_UI.SetEdit(m_wRange, Arg(payload, 3));
				return;
			}
		}

		Status(Arg(payload, 0));
		Request0("net_info");
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case 1: Request0("net_scan"); return true;
			case 3:
			{
				int i = m_List.m_iSelected;
				if (i >= 0 && i < m_aGateways.Count())
					Request2("net_connect", m_aGateways[i], m_wPassword.GetText());

				return true;
			}

			case 4: Request0("net_disconnect"); return true;
			case 6: Request1("net_setip", m_wIp.GetText()); return true;
			case 7: Request0("net_ssh_toggle"); return true;
			case 9: Request1("router_page", m_wRouterPw.GetText()); return true;
			case 13:
			{
				array<string> args = {m_wRouterPw.GetText(), m_wSsid.GetText(), m_wNewPw.GetText(), m_wRange.GetText()};
				Request("router_set", args);
				return true;
			}

			case 14: Request1("router_ext_ssh", m_wRouterPw.GetText()); return true;
		}

		return false;
	}
}
