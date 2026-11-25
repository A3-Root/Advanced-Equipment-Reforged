//! Widgets and events of the ROOT_AE3_GmDialog action panel: one column of buttons per action group.
class ROOT_AE3_GmPanel : ROOT_AE3_UiListener
{
	protected const int ID_CLOSE = 1;
	protected const int ID_ACTION = 1000;
	protected const float COLUMN = 220;
	protected const float ROW = 32;

	protected ROOT_AE3_GmDialog m_Dialog;
	protected ref array<string> m_aKeys = {};

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_GmPanel(ROOT_AE3_GmDialog dialog, Widget root, string devices, array<string> applicable)
	{
		m_Dialog = dialog;
		array<string> keys = {};
		array<string> labels = {};
		array<string> groups = {};
		ROOT_AE3_GmActions.List(keys, labels, groups);

		array<string> columns = {};
		foreach (int i, string key : keys)
		{
			if (applicable.Contains(key) && !columns.Contains(groups[i]))
				columns.Insert(groups[i]);
		}

		int rows;
		foreach (string group : columns)
		{
			int n;
			foreach (int i, string key : keys)
			{
				if (groups[i] == group && applicable.Contains(key))
					n++;
			}

			rows = Math.Max(rows, n);
		}

		float sw, sh;
		ROOT_AE3_UI.ScreenSize(sw, sh);
		float w = Math.Max(420, 16 + columns.Count() * (COLUMN + 10));
		float h = 110 + rows * ROW;
		float x = (sw - w) * 0.5;
		float y = (sh - h) * 0.5;
		ROOT_AE3_UI.Rect(root, 0, 0, sw, sh, ROOT_AE3_UI.Rgb(0, 0, 0, 0.4));
		ROOT_AE3_UI.Rect(root, x, y, w, h, ROOT_AE3_UI.Rgb(0.12, 0.13, 0.15, 0.97));
		ROOT_AE3_UI.Rect(root, x, y, w, 4, ROOT_AE3_UI.Rgb(0.91, 0.33, 0.13));
		ROOT_AE3_UI.Text(root, x + 16, y + 14, w - 140, 26, "Advanced Equipment - " + devices, 20);
		ROOT_AE3_UI.Button(root, x + w - 116, y + 12, 100, 28, "Close", ID_CLOSE, this);
		if (columns.IsEmpty())
		{
			ROOT_AE3_UI.Text(root, x + 16, y + 56, w - 32, 22, "No action applies to the selection.", 15, ROOT_AE3_UI.Rgb(0.75, 0.75, 0.78));
			return;
		}

		foreach (int c, string group : columns)
		{
			float cx = x + 16 + c * (COLUMN + 10);
			ROOT_AE3_UI.Text(root, cx, y + 50, COLUMN, 22, group, 15, ROOT_AE3_UI.Rgb(0.91, 0.33, 0.13));
			int r;
			foreach (int i, string key : keys)
			{
				if (groups[i] != group || !applicable.Contains(key))
					continue;

				m_aKeys.Insert(key);
				ROOT_AE3_UI.Button(root, cx, y + 76 + r * ROW, COLUMN, ROW - 4, labels[i], ID_ACTION + m_aKeys.Count() - 1, this, null, null, 14);
				r++;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == ID_CLOSE)
		{
			m_Dialog.Close();
			return true;
		}

		int index = id - ID_ACTION;
		if (index >= 0 && index < m_aKeys.Count())
		{
			// Deferred: Choose removes the clicked widget.
			GetGame().GetCallqueue().CallLater(m_Dialog.Choose, 0, false, m_aKeys[index]);
			return true;
		}

		return false;
	}
}
