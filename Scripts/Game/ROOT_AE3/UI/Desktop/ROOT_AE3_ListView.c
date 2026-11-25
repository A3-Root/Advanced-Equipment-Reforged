//! Scrollable list with columns. Click selects, double click activates; both reported to the owner app.
class ROOT_AE3_ListView : ROOT_AE3_UiListener
{
	protected static const int ID_ROW = 1000;
	protected static const int ID_AREA = 999;
	protected static const float ROW_H = 24;

	ROOT_AE3_App m_Owner;
	int m_iListId;
	ref array<string> m_aHeaders = {};
	ref array<float> m_aWidths = {};
	ref array<ref array<string>> m_aRows = {};
	int m_iSelected = -1;
	protected int m_iScroll;
	protected FrameWidget m_wFrame;
	protected float m_fW, m_fH;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_ListView(ROOT_AE3_App owner, int listId, Widget parent, float x, float y, float w, float h)
	{
		m_Owner = owner;
		m_iListId = listId;
		m_fW = w;
		m_fH = h;
		m_wFrame = ROOT_AE3_UI.Frame(parent, x, y, w, h);
	}

	//------------------------------------------------------------------------------------------------
	void SetColumns(notnull array<string> headers, notnull array<float> widths)
	{
		m_aHeaders.Copy(headers);
		m_aWidths.Copy(widths);
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_aRows.Clear();
		m_iSelected = -1;
		m_iScroll = 0;
	}

	//------------------------------------------------------------------------------------------------
	void AddRow(notnull array<string> cells)
	{
		array<string> r = {};
		r.Copy(cells);
		m_aRows.Insert(r);
	}

	//------------------------------------------------------------------------------------------------
	array<string> Selected()
	{
		if (m_iSelected < 0 || m_iSelected >= m_aRows.Count())
			return null;

		return m_aRows[m_iSelected];
	}

	//------------------------------------------------------------------------------------------------
	protected int Visible()
	{
		int head = 0;
		if (!m_aHeaders.IsEmpty())
			head = 1;

		return Math.Max(1, Math.Floor(m_fH / ROW_H) - head);
	}

	//------------------------------------------------------------------------------------------------
	void Render()
	{
		ReleaseHandlers();
		ROOT_AE3_UI.Clear(m_wFrame);
		ROOT_AE3_UI.Rect(m_wFrame, 0, 0, m_fW, m_fH, ROOT_AE3_Theme.Field(), ID_AREA, this);
		float y = 0;
		if (!m_aHeaders.IsEmpty())
		{
			ROOT_AE3_UI.Rect(m_wFrame, 0, 0, m_fW, ROW_H, ROOT_AE3_Theme.Bar());
			float hx = 6;
			foreach (int i, string h : m_aHeaders)
			{
				ROOT_AE3_UI.Text(m_wFrame, hx, 3, m_aWidths[i] - 6, ROW_H - 4, h, 14, ROOT_AE3_Theme.Muted());
				hx += m_aWidths[i];
			}

			y = ROW_H;
		}

		int vis = Visible();
		m_iScroll = Math.ClampInt(m_iScroll, 0, Math.Max(0, m_aRows.Count() - vis));
		for (int r = m_iScroll; r < Math.Min(m_aRows.Count(), m_iScroll + vis); r++)
		{
			Color bg = ROOT_AE3_UI.Rgb(0, 0, 0, 0);
			if (r == m_iSelected)
				bg = ROOT_AE3_Theme.Select();

			ROOT_AE3_UI.Rect(m_wFrame, 0, y, m_fW, ROW_H, bg, ID_ROW + r, this);
			float cx = 6;
			array<string> row = m_aRows[r];
			for (int c = 0; c < row.Count() && c < Math.Max(1, m_aWidths.Count()); c++)
			{
				float cw = m_fW;
				if (c < m_aWidths.Count())
					cw = m_aWidths[c];

				ROOT_AE3_UI.Text(m_wFrame, cx, y + 3, cw - 8, ROW_H - 4, row[c], 15, ROOT_AE3_Theme.Text());
				cx += cw;
			}

			y += ROW_H;
		}

		if (m_aRows.Count() > vis)
			ROOT_AE3_UI.Text(m_wFrame, m_fW - 70, m_fH - 18, 66, 16, string.Format("%1/%2", m_iScroll + vis, m_aRows.Count()), 12, ROOT_AE3_Theme.Muted());
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id < ID_ROW)
			return false;

		m_iSelected = id - ID_ROW;
		Render();
		m_Owner.OnListSelect(m_iListId, m_iSelected);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiDoubleClick(Widget w, int id)
	{
		if (id < ID_ROW)
			return false;

		m_iSelected = id - ID_ROW;
		m_Owner.OnListActivate(m_iListId, m_iSelected);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiWheel(Widget w, int id, int wheel)
	{
		m_iScroll -= wheel * 2;
		Render();
		return true;
	}
}
