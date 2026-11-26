//! Network map: schematic of the devices around this computer.
class ROOT_AE3_AppMap : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Map";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "map";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(720, 560, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected FrameWidget m_wMap;
	protected float m_fTimer;

	override void Build()
	{
		m_wMap = ROOT_AE3_UI.Frame(GetContent(), 6, 6, Width() - 12, Height() - 12);
		Request0("map_data");
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		m_fTimer += dt;
		if (m_fTimer > 5)
		{
			m_fTimer = 0;
			Request0("map_data");
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok || !m_wMap)
			return;

		ReleaseHandlers();
		ROOT_AE3_UI.Clear(m_wMap);
		float w = FrameSlot.GetSizeX(m_wMap);
		float h = FrameSlot.GetSizeY(m_wMap);
		ROOT_AE3_UI.Rect(m_wMap, 0, 0, w, h, ROOT_AE3_UI.Rgb(0.06, 0.1, 0.08));
		for (int g = 1; g < 10; g++)
		{
			ROOT_AE3_UI.Rect(m_wMap, w * g / 10, 0, 1, h, ROOT_AE3_UI.Rgb(0.2, 0.35, 0.25, 0.5));
			ROOT_AE3_UI.Rect(m_wMap, 0, h * g / 10, w, 1, ROOT_AE3_UI.Rgb(0.2, 0.35, 0.25, 0.5));
		}

		float range = Arg(payload, 0).ToFloat();
		if (range <= 0)
			range = 500;

		float cx = w * 0.5;
		float cy = h * 0.5;
		for (int i = 1; i < payload.Count(); i++)
		{
			array<string> e = ROOT_AE3_Codec.SplitSub(payload[i]);
			if (e.Count() < 5)
				continue;

			float dx = e[2].ToFloat() / range * w * 0.5;
			float dy = e[3].ToFloat() / range * h * 0.5;
			float x = cx + dx;
			float y = cy - dy;
			if (x < 0 || y < 0 || x > w || y > h)
				continue;

			Color c = ROOT_AE3_UI.Rgb(0.4, 0.9, 0.4);
			if (e[1] == "router")
				c = ROOT_AE3_UI.Rgb(0.3, 0.6, 1);
			else if (e[1] == "self")
				c = ROOT_AE3_UI.Rgb(1, 0.5, 0.2);
			else if (e[1] == "power")
				c = ROOT_AE3_UI.Rgb(1, 0.9, 0.3);

			ROOT_AE3_UI.Rect(m_wMap, x - 5, y - 5, 10, 10, c);
			ROOT_AE3_UI.Text(m_wMap, x + 8, y - 9, 220, 18, e[0] + "  " + e[4], 13, c);
		}

		ROOT_AE3_UI.Text(m_wMap, 8, h - 22, w - 16, 18, string.Format("Grid %1 m. Orange: this computer, blue: routers, green: computers, yellow: power.", Math.Round(range / 5)), 13);
	}
}
