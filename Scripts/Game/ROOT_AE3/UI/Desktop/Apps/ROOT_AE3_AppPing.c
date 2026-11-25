//! Ping an address.
class ROOT_AE3_AppPing : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Ping";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "connection";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(480, 320, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected EditBoxWidget m_wIp;
	protected TextWidget m_wOut;

	override void Build()
	{
		float w = Width();
		m_wIp = Field(6, 6, w - 100, "192.168.1.1", 1);
		Btn(w - 88, 6, 82, 26, "Ping", 2);
		m_wOut = Label(10, 42, w - 20, Height() - 50, "", 15, true, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		m_wOut.SetText(Arg(payload, 0));
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == 2)
		{
			m_wOut.SetText("Pinging...");
			Request1("ping_host", m_wIp.GetText());
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (finished && id == 1)
			return OnUiClick(w, 2, 0);

		return false;
	}
}
