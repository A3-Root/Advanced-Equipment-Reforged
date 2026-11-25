//! System information: host, user, power, battery, network.
class ROOT_AE3_AppSysInfo : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "System Info";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "system";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(480, 320, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected TextWidget m_wText;
	protected float m_fTimer;

	override void Build()
	{
		m_wText = Label(14, 12, Width() - 28, Height() - 24, "Loading...", 15, true, true);
		Request0("sysinfo");
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		m_fTimer += dt;
		if (m_fTimer > 3)
		{
			m_fTimer = 0;
			Request0("sysinfo");
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (ok && m_wText)
			m_wText.SetText(Arg(payload, 0));
	}
}
