//! Device map around this computer. Mouse wheel or +/- zoom at the cursor, drag pans, arrow keys pan,
//! Center returns to this computer. Devices are refreshed every 5 s.
class ROOT_AE3_AppMap : ROOT_AE3_App
{
	protected static const int ID_MAP = 1;
	protected static const int ID_ZOOM_IN = 2;
	protected static const int ID_ZOOM_OUT = 3;
	protected static const int ID_CENTER = 4;
	protected static const float MIN_SPAN = 20;		//!< Smallest visible half width in m
	protected static const float MAX_SPAN = 5000;
	protected static const float TOP = 40;

	protected FrameWidget m_wMap;
	protected ref ROOT_AE3_ForwardListener m_MapListener;
	protected ref array<ref array<string>> m_aDevices = {};
	protected float m_fSpan = 250;		//!< Half width of the view in m
	protected float m_fCenterX;			//!< View center, m east of this computer
	protected float m_fCenterY;			//!< View center, m north of this computer
	protected float m_fFetched;			//!< Range of the last request
	protected float m_fTimer;
	protected bool m_bDrag;
	protected float m_fDragX;
	protected float m_fDragY;
	protected float m_fMapW;
	protected float m_fMapH;

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
		return Vector(760, 580, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		float w = Width();
		float h = Height();
		IconBtn(8, "g_plus", ID_ZOOM_IN);
		IconBtn(44, "g_min", ID_ZOOM_OUT);
		Btn(84, 6, 90, 28, "Center", ID_CENTER);
		ROOT_AE3_UI.Text(GetContent(), 184, 12, w - 192, 18, "Wheel: zoom   Drag: move   Arrow keys: move", 13, ROOT_AE3_Theme.Muted(), false, false, 2);
		m_fMapW = w;
		m_fMapH = h - TOP;
		m_wMap = ROOT_AE3_UI.Frame(GetContent(), 0, TOP, m_fMapW, m_fMapH);
		Fetch();
		Render();
	}

	//------------------------------------------------------------------------------------------------
	protected void IconBtn(float x, string icon, int id)
	{
		ROOT_AE3_UI.Box(GetContent(), x, 6, 30, 28, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.IconButton(GetContent(), x + 1, 7, 28, 26, icon, id, this, 14, ROOT_AE3_Theme.Text(), ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Hover());
	}

	//------------------------------------------------------------------------------------------------
	protected void Fetch()
	{
		float need = m_fSpan * 1.5 + Math.Max(Math.AbsFloat(m_fCenterX), Math.AbsFloat(m_fCenterY));
		m_fFetched = Math.Clamp(need, 100, MAX_SPAN);
		Request1("map_data", Math.Round(m_fFetched).ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! Meters per reference unit.
	protected float Scale()
	{
		return m_fSpan * 2 / m_fMapW;
	}

	//------------------------------------------------------------------------------------------------
	//! Grid step in m: 1, 2 or 5 times a power of ten, about 80 units apart.
	protected float GridStep()
	{
		float raw = Scale() * 80;
		float p = 1;
		while (p * 10 <= raw)
		{
			p *= 10;
		}

		if (raw >= p * 5)
			return p * 5;

		if (raw >= p * 2)
			return p * 2;

		return p;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Distance(float m)
	{
		if (m >= 1000)
			return string.Format("%1 km", m / 1000);

		return string.Format("%1 m", Math.Round(m));
	}

	//------------------------------------------------------------------------------------------------
	protected void Render()
	{
		if (!m_wMap)
			return;

		m_MapListener = new ROOT_AE3_ForwardListener(this);
		ROOT_AE3_UI.Clear(m_wMap);
		float w = m_fMapW;
		float h = m_fMapH;
		ROOT_AE3_UI.Rect(m_wMap, 0, 0, w, h, ROOT_AE3_UI.Rgb(0.07, 0.11, 0.09), ID_MAP, m_MapListener);
		float scale = Scale();
		float step = GridStep();
		float left = m_fCenterX - w * 0.5 * scale;
		float top = m_fCenterY + h * 0.5 * scale;
		float gx = Math.Floor(left / step) * step;
		while (gx < left + w * scale)
		{
			float sx = (gx - left) / scale;
			float a = 0.25;
			if (Math.AbsFloat(gx) < step * 0.5)
				a = 0.6;

			ROOT_AE3_UI.Rect(m_wMap, sx, 0, 1, h, ROOT_AE3_UI.Rgb(0.3, 0.5, 0.35, a));
			gx += step;
		}

		float gy = Math.Floor((top - h * scale) / step) * step;
		while (gy < top)
		{
			float sy = (top - gy) / scale;
			float b = 0.25;
			if (Math.AbsFloat(gy) < step * 0.5)
				b = 0.6;

			ROOT_AE3_UI.Rect(m_wMap, 0, sy, w, 1, ROOT_AE3_UI.Rgb(0.3, 0.5, 0.35, b));
			gy += step;
		}

		foreach (array<string> e : m_aDevices)
		{
			float x = (e[2].ToFloat() - left) / scale;
			float y = (top - e[3].ToFloat()) / scale;
			if (x < -20 || y < -20 || x > w + 20 || y > h + 20)
				continue;

			string icon = "computer";
			Color c = ROOT_AE3_UI.Rgb(0.45, 0.95, 0.45);
			if (e[1] == "router")
			{
				icon = "network";
				c = ROOT_AE3_UI.Rgb(0.45, 0.7, 1);
			}
			else if (e[1] == "self")
			{
				icon = "logo";
				c = ROOT_AE3_UI.Rgb(1, 0.6, 0.25);
			}
			else if (e[1] == "power")
			{
				icon = "g_plug";
				c = ROOT_AE3_UI.Rgb(1, 0.9, 0.35);
			}

			ROOT_AE3_UI.Rect(m_wMap, x - 12, y - 12, 24, 24, ROOT_AE3_UI.Rgb(0, 0, 0, 0.45));
			if (icon == "g_plug")
				ROOT_AE3_UI.Icon(m_wMap, x - 9, y - 9, 18, icon, c);
			else
				ROOT_AE3_UI.Icon(m_wMap, x - 10, y - 10, 20, icon);

			string label = e[0];
			if (!e[4].IsEmpty())
				label += "  " + e[4];

			TextWidget t = ROOT_AE3_UI.Text(m_wMap, x + 15, y - 9, 260, 18, label, 13, c);
			t.SetShadow(2, 0xFF000000, 0.9, 0, 1);
		}

		// Scale bar and legend.
		float barW = step / scale;
		ROOT_AE3_UI.Rect(m_wMap, 12, h - 44, barW, 3, ROOT_AE3_UI.Rgb(1, 1, 1, 0.9));
		ROOT_AE3_UI.Rect(m_wMap, 12, h - 50, 1, 9, ROOT_AE3_UI.Rgb(1, 1, 1, 0.9));
		ROOT_AE3_UI.Rect(m_wMap, 12 + barW, h - 50, 1, 9, ROOT_AE3_UI.Rgb(1, 1, 1, 0.9));
		ROOT_AE3_UI.Text(m_wMap, 18 + barW, h - 54, 120, 18, Distance(step), 13, ROOT_AE3_UI.Rgb(1, 1, 1));
		ROOT_AE3_UI.Rect(m_wMap, 0, h - 26, w, 26, ROOT_AE3_UI.Rgb(0, 0, 0, 0.5));
		ROOT_AE3_UI.Text(m_wMap, 10, h - 22, w - 20, 18, "Orange: this computer   Blue: routers   Green: computers   Yellow: power", 13, ROOT_AE3_UI.Rgb(0.85, 0.85, 0.85));
		ROOT_AE3_UI.Text(m_wMap, w - 60, 8, 50, 18, "N ^", 14, ROOT_AE3_UI.Rgb(1, 1, 1), false, false, 2);
	}

	//------------------------------------------------------------------------------------------------
	//! Zooms by factor around the map point under (lx, ly) in map units.
	protected void Zoom(float factor, float lx, float ly)
	{
		float scale = Scale();
		float wx = m_fCenterX + (lx - m_fMapW * 0.5) * scale;
		float wy = m_fCenterY - (ly - m_fMapH * 0.5) * scale;
		m_fSpan = Math.Clamp(m_fSpan * factor, MIN_SPAN, MAX_SPAN);
		scale = Scale();
		m_fCenterX = wx - (lx - m_fMapW * 0.5) * scale;
		m_fCenterY = wy + (ly - m_fMapH * 0.5) * scale;
		Changed();
	}

	//------------------------------------------------------------------------------------------------
	protected void Pan(float dx, float dy)
	{
		m_fCenterX = Math.Clamp(m_fCenterX + dx, -MAX_SPAN, MAX_SPAN);
		m_fCenterY = Math.Clamp(m_fCenterY + dy, -MAX_SPAN, MAX_SPAN);
		Changed();
	}

	//------------------------------------------------------------------------------------------------
	protected void Changed()
	{
		float need = m_fSpan * 1.2 + Math.Max(Math.AbsFloat(m_fCenterX), Math.AbsFloat(m_fCenterY));
		if (need > m_fFetched && m_fFetched < MAX_SPAN)
			Fetch();

		Render();
	}

	//------------------------------------------------------------------------------------------------
	//! Mouse position in map units.
	protected void MouseOnMap(out float x, out float y)
	{
		float mx, my;
		ROOT_AE3_UI.Mouse(mx, my);
		float sx, sy;
		m_wMap.GetScreenPos(sx, sy);
		WorkspaceWidget ws = GetGame().GetWorkspace();
		x = mx - ws.DPIUnscale(sx);
		y = my - ws.DPIUnscale(sy);
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		m_fTimer += dt;
		if (m_fTimer > 5)
		{
			m_fTimer = 0;
			Fetch();
		}

		if (!m_bDrag)
			return;

		float mx, my;
		ROOT_AE3_UI.Mouse(mx, my);
		if (GetGame().GetInputManager().GetActionValue("MouseLeft") <= 0)
			m_bDrag = false;

		float dx = mx - m_fDragX;
		float dy = my - m_fDragY;
		if (dx == 0 && dy == 0)
			return;

		m_fDragX = mx;
		m_fDragY = my;
		Pan(-dx * Scale(), dy * Scale());
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		m_aDevices.Clear();
		for (int i = 1; i < payload.Count(); i++)
		{
			array<string> e = ROOT_AE3_Codec.SplitSub(payload[i]);
			if (e.Count() >= 5)
				m_aDevices.Insert(e);
		}

		Status(string.Format("%1 devices", m_aDevices.Count()));
		if (!m_bDrag)
			Render();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnKey(KeyCode key, bool ctrl)
	{
		float d = m_fSpan * 0.2;
		switch (key)
		{
			case KeyCode.KC_LEFT: Pan(-d, 0); return true;
			case KeyCode.KC_RIGHT: Pan(d, 0); return true;
			case KeyCode.KC_UP: Pan(0, d); return true;
			case KeyCode.KC_DOWN: Pan(0, -d); return true;
			case KeyCode.KC_ADD: Zoom(0.7, m_fMapW * 0.5, m_fMapH * 0.5); return true;
			case KeyCode.KC_EQUALS: Zoom(0.7, m_fMapW * 0.5, m_fMapH * 0.5); return true;
			case KeyCode.KC_SUBTRACT: Zoom(1 / 0.7, m_fMapW * 0.5, m_fMapH * 0.5); return true;
			case KeyCode.KC_MINUS: Zoom(1 / 0.7, m_fMapW * 0.5, m_fMapH * 0.5); return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseDown(Widget w, int id, int button)
	{
		if (id != ID_MAP || button != 0)
			return false;

		m_bDrag = true;
		ROOT_AE3_UI.Mouse(m_fDragX, m_fDragY);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiMouseUp(Widget w, int id, int button)
	{
		if (button == 0)
			m_bDrag = false;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiWheel(Widget w, int id, int wheel)
	{
		if (id != ID_MAP)
			return false;

		float lx, ly;
		MouseOnMap(lx, ly);
		float factor = 0.8;
		if (wheel < 0)
			factor = 1.25;

		Zoom(factor, lx, ly);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case ID_ZOOM_IN: Zoom(0.7, m_fMapW * 0.5, m_fMapH * 0.5); return true;
			case ID_ZOOM_OUT: Zoom(1 / 0.7, m_fMapW * 0.5, m_fMapH * 0.5); return true;
			case ID_CENTER:
			{
				m_fCenterX = 0;
				m_fCenterY = 0;
				Changed();
				return true;
			}

			case ID_MAP: return true;
		}

		return false;
	}
}
