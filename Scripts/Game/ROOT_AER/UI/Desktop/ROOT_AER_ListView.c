//! Scrollable list: details (rows with columns) or icons (grid). Click selects, double click activates,
//! right click selects and asks the owner for a context menu, header click asks for sorting.
class ROOT_AER_ListView : ROOT_AER_UiListener
{
	protected static const int ID_AREA = 999;
	protected static const int ID_HEAD = 900;
	protected static const int ID_ROW = 1000;
	protected static const float ROW_H = 28;
	protected static const float CELL_W = 100;
	protected static const float CELL_H = 96;

	ROOT_AER_App m_Owner;
	int m_iListId;
	ref array<string> m_aHeaders = {};
	ref array<float> m_aWidths = {};
	ref array<ref array<string>> m_aRows = {};
	ref array<string> m_aIcons = {};
	int m_iSelected = -1;
	bool m_bGrid;
	int m_iSortCol = -1;
	bool m_bSortDesc;
	string m_sEmpty = "";
	protected int m_iScroll;
	protected FrameWidget m_wFrame;
	protected float m_fW, m_fH;

	//------------------------------------------------------------------------------------------------
	void ROOT_AER_ListView(ROOT_AER_App owner, int listId, Widget parent, float x, float y, float w, float h)
	{
		m_Owner = owner;
		m_iListId = listId;
		m_fW = w;
		m_fH = h;
		m_wFrame = ROOT_AER_UI.Frame(parent, x, y, w, h);
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
		m_aIcons.Clear();
		m_iSelected = -1;
		m_iScroll = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! icon: ROOT_AER_Icons name shown in the first column and in the grid.
	void AddRow(notnull array<string> cells, string icon = "")
	{
		array<string> r = {};
		r.Copy(cells);
		m_aRows.Insert(r);
		m_aIcons.Insert(icon);
	}

	//------------------------------------------------------------------------------------------------
	array<string> Selected()
	{
		if (m_iSelected < 0 || m_iSelected >= m_aRows.Count())
			return null;

		return m_aRows[m_iSelected];
	}

	//------------------------------------------------------------------------------------------------
	protected int Columns()
	{
		if (!m_bGrid)
			return 1;

		return Math.Max(1, Math.Floor((m_fW - 12) / CELL_W));
	}

	//------------------------------------------------------------------------------------------------
	//! Visible lines (rows, or grid rows).
	protected int Lines()
	{
		if (m_bGrid)
			return Math.Max(1, Math.Floor((m_fH - 8) / CELL_H));

		float top = 0;
		if (!m_aHeaders.IsEmpty())
			top = ROW_H;

		return Math.Max(1, Math.Floor((m_fH - top) / ROW_H));
	}

	//------------------------------------------------------------------------------------------------
	protected int TotalLines()
	{
		int cols = Columns();
		return (m_aRows.Count() + cols - 1) / cols;
	}

	//------------------------------------------------------------------------------------------------
	//! Moves the selection by delta rows (grid: delta * columns for vertical moves) and keeps it visible.
	void Move(int delta, bool vertical = true)
	{
		if (m_aRows.IsEmpty())
			return;

		int step = delta;
		if (vertical)
			step = delta * Columns();

		m_iSelected = Math.ClampInt(m_iSelected + step, 0, m_aRows.Count() - 1);
		int line = m_iSelected / Columns();
		if (line < m_iScroll)
			m_iScroll = line;
		else if (line >= m_iScroll + Lines())
			m_iScroll = line - Lines() + 1;

		Render();
		m_Owner.OnListSelect(m_iListId, m_iSelected);
	}

	//------------------------------------------------------------------------------------------------
	void Render()
	{
		ReleaseHandlers();
		ROOT_AER_UI.Clear(m_wFrame);
		ROOT_AER_UI.Rect(m_wFrame, 0, 0, m_fW, m_fH, ROOT_AER_Theme.Field(), ID_AREA, this);
		int lines = Lines();
		m_iScroll = Math.ClampInt(m_iScroll, 0, Math.Max(0, TotalLines() - lines));
		if (m_bGrid)
			RenderGrid(lines);
		else
			RenderDetails(lines);

		if (m_aRows.IsEmpty() && !m_sEmpty.IsEmpty())
			ROOT_AER_UI.Text(m_wFrame, 0, m_fH * 0.4, m_fW, 20, m_sEmpty, 14, ROOT_AER_Theme.Muted(), false, false, 1);

		int total = TotalLines();
		if (total > lines)
		{
			float top = 0;
			if (!m_bGrid && !m_aHeaders.IsEmpty())
				top = ROW_H;

			float track = m_fH - top - 4;
			float thumb = Math.Max(24, track * lines / total);
			float pos = (track - thumb) * m_iScroll / Math.Max(1, total - lines);
			ROOT_AER_UI.Rect(m_wFrame, m_fW - 6, top + 2 + pos, 4, thumb, ROOT_AER_Theme.Muted());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RenderDetails(int lines)
	{
		float y = 0;
		if (!m_aHeaders.IsEmpty())
		{
			ROOT_AER_UI.Rect(m_wFrame, 0, ROW_H - 1, m_fW, 1, ROOT_AER_Theme.Border());
			float hx = 0;
			foreach (int i, string h : m_aHeaders)
			{
				string label = h;
				if (i == m_iSortCol)
				{
					if (m_bSortDesc)
						label += "  v";
					else
						label += "  ^";
				}

				ROOT_AER_UI.Hit(m_wFrame, hx, 0, m_aWidths[i], ROW_H - 1, ID_HEAD + i, this, ROOT_AER_UI.Rgb(0, 0, 0, 0));
				ROOT_AER_UI.Text(m_wFrame, hx + 10, 6, m_aWidths[i] - 14, ROW_H - 8, label, 13, ROOT_AER_Theme.Muted());
				if (i > 0)
					ROOT_AER_UI.Rect(m_wFrame, hx, 5, 1, ROW_H - 10, ROOT_AER_Theme.Border());

				hx += m_aWidths[i];
			}

			y = ROW_H;
		}

		for (int r = m_iScroll; r < Math.Min(m_aRows.Count(), m_iScroll + lines); r++)
		{
			Color bg = ROOT_AER_UI.Rgb(0, 0, 0, 0);
			if (r == m_iSelected)
				bg = ROOT_AER_Theme.Select();

			ROOT_AER_UI.Hit(m_wFrame, 2, y + 1, m_fW - 4, ROW_H - 2, ID_ROW + r, this, bg, ROOT_AER_Theme.Hover());
			float cx = 10;
			array<string> row = m_aRows[r];
			for (int c = 0; c < row.Count() && c < Math.Max(1, m_aWidths.Count()); c++)
			{
				float cw = m_fW;
				if (c < m_aWidths.Count())
					cw = m_aWidths[c];

				float tx = cx;
				if (c == 0 && !m_aIcons[r].IsEmpty())
				{
					ROOT_AER_UI.Icon(m_wFrame, cx, y + 5, 18, m_aIcons[r]);
					tx = cx + 26;
				}

				Color col = ROOT_AER_Theme.Text();
				if (c > 0)
					col = ROOT_AER_Theme.Muted();

				ROOT_AER_UI.Text(m_wFrame, tx, y + 6, cw - (tx - cx) - 12, ROW_H - 8, row[c], 14, col);
				cx += cw;
			}

			y += ROW_H;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RenderGrid(int lines)
	{
		int cols = Columns();
		int first = m_iScroll * cols;
		int last = Math.Min(m_aRows.Count(), first + lines * cols);
		for (int r = first; r < last; r++)
		{
			int k = r - first;
			int gx = k % cols;
			int gy = k / cols;
			float x = 6 + gx * CELL_W;
			float y = 4 + gy * CELL_H;
			Color bg = ROOT_AER_UI.Rgb(0, 0, 0, 0);
			if (r == m_iSelected)
				bg = ROOT_AER_Theme.Select();

			ROOT_AER_UI.Hit(m_wFrame, x + 2, y + 2, CELL_W - 4, CELL_H - 4, ID_ROW + r, this, bg, ROOT_AER_Theme.Hover());
			string icon = m_aIcons[r];
			if (icon.IsEmpty())
				icon = "file";

			ROOT_AER_UI.Icon(m_wFrame, x + (CELL_W - 52) * 0.5, y + 6, 52, icon);
			ROOT_AER_UI.Text(m_wFrame, x + 4, y + 60, CELL_W - 8, 32, m_aRows[r][0], 13, ROOT_AER_Theme.Text(), false, true, 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= ID_HEAD && id < ID_HEAD + m_aHeaders.Count())
		{
			if (button == 0)
				m_Owner.OnListSort(m_iListId, id - ID_HEAD);

			return true;
		}

		if (id == ID_AREA)
		{
			if (m_iSelected >= 0)
			{
				m_iSelected = -1;
				Render();
				m_Owner.OnListSelect(m_iListId, -1);
			}

			if (button == 1)
				m_Owner.OnListContext(m_iListId, -1);

			return true;
		}

		if (id < ID_ROW)
			return false;

		m_iSelected = id - ID_ROW;
		Render();
		m_Owner.OnListSelect(m_iListId, m_iSelected);
		if (button == 1)
			m_Owner.OnListContext(m_iListId, m_iSelected);

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
		int step = 3;
		if (m_bGrid)
			step = 1;

		m_iScroll -= wheel * step;
		Render();
		return true;
	}
}
