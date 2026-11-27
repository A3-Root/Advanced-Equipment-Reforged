//! Widgets and events of the ROOT_AE3_GmDialog field form: text, multiline, password, on / off, choice and
//! permission fields. Fields with a ShowIf condition appear when the deciding field has a matching value.
class ROOT_AE3_GmDialogForm : ROOT_AE3_UiListener
{
	protected static const int ID_OK = 1;
	protected static const int ID_CANCEL = 2;
	protected static const int ID_EDIT = 100;
	protected static const int ID_FIELD = 1000;	//!< + field * 100 + part
	protected static const int PART_PREV = 1;
	protected static const int PART_NEXT = 2;
	protected static const int PART_OPTION = 10;
	protected static const int PART_PERM = 50;
	protected static const float WIDTH = 720;
	protected static const float LABEL = 230;
	protected static const float ROW = 30;
	protected static const float ROW_MULTI = 96;
	protected static const float GAP = 8;
	protected static const float FIELD = WIDTH - LABEL - 40;
	protected static const int MAX_COLUMNS = 4;
	protected static const int MAX_GRID = 20;	//!< More choices: arrows

	protected ROOT_AE3_GmDialog m_Dialog;
	protected Widget m_wRoot;
	protected string m_sTitle;
	protected string m_sOk;
	protected ref array<ref ROOT_AE3_GmField> m_aFields = {};
	protected ref map<int, Widget> m_mEdits = new map<int, Widget>();
	protected ref array<int> m_aEditOrder = {};

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_GmDialogForm(ROOT_AE3_GmDialog dialog, Widget root, string title, notnull array<ref ROOT_AE3_GmField> fields, string okLabel = "OK")
	{
		m_Dialog = dialog;
		m_wRoot = root;
		m_sTitle = title;
		m_sOk = okLabel;
		foreach (ROOT_AE3_GmField f : fields)
		{
			m_aFields.Insert(f);
		}

		Build(true);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Visible(int i, int depth = 0)
	{
		ROOT_AE3_GmField f = m_aFields[i];
		if (f.m_iShowIf < 0 || f.m_iShowIf >= m_aFields.Count() || f.m_iShowIf == i || depth > 8)
			return true;

		return f.m_aShowValues.Contains(m_aFields[f.m_iShowIf].m_sValue) && Visible(f.m_iShowIf, depth + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected static float RowHeight(ROOT_AE3_GmField f)
	{
		if (f.m_eKind == ROOT_AE3_EGmField.MULTILINE)
			return ROW_MULTI;

		int cols = Columns(f);
		if (cols <= 0)
			return ROW;

		int rows = (f.m_aValues.Count() + cols - 1) / cols;
		return rows * ROW + (rows - 1) * 4;
	}

	//------------------------------------------------------------------------------------------------
	//! Button columns of a choice field so the longest label fits; 0 = not a grid (arrows or no choice).
	protected static int Columns(ROOT_AE3_GmField f)
	{
		int n = f.m_aValues.Count();
		if (f.m_eKind != ROOT_AE3_EGmField.CHOICE || n == 0 || n > MAX_GRID)
			return 0;

		int longest = 0;
		foreach (string l : f.m_aLabels)
		{
			longest = Math.Max(longest, l.Length());
		}

		float need = longest * 9 + 16;
		for (int c = Math.Min(n, MAX_COLUMNS); c > 1; c--)
		{
			if ((FIELD - (c - 1) * 4) / c >= need)
				return c;
		}

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Copies the text of the edit widgets into the field values.
	protected void ReadEdits()
	{
		foreach (int i, Widget w : m_mEdits)
		{
			if (!w || i >= m_aFields.Count())
				continue;

			EditBoxWidget e = EditBoxWidget.Cast(w);
			if (e)
				m_aFields[i].m_sValue = e.GetText();

			MultilineEditBoxWidget m = MultilineEditBoxWidget.Cast(w);
			if (m)
				m_aFields[i].m_sValue = m.GetText();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Rebuild()
	{
		Build(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Build(bool focus)
	{
		ReleaseHandlers();
		ROOT_AE3_UI.Clear(m_wRoot);
		m_mEdits.Clear();
		m_aEditOrder.Clear();

		float sw, sh;
		ROOT_AE3_UI.ScreenSize(sw, sh);
		float h = 104;
		foreach (int i, ROOT_AE3_GmField f : m_aFields)
		{
			if (Visible(i))
				h += RowHeight(f) + GAP;
		}

		float x = (sw - WIDTH) * 0.5;
		float y = Math.Max(10, (sh - h) * 0.5);
		Color accent = ROOT_AE3_UI.Rgb(0.91, 0.33, 0.13);
		ROOT_AE3_UI.Rect(m_wRoot, 0, 0, sw, sh, ROOT_AE3_UI.Rgb(0, 0, 0, 0.4));
		ROOT_AE3_UI.Rect(m_wRoot, x, y, WIDTH, h, ROOT_AE3_UI.Rgb(0.12, 0.13, 0.15, 0.97));
		ROOT_AE3_UI.Rect(m_wRoot, x, y, WIDTH, 4, accent);
		ROOT_AE3_UI.Text(m_wRoot, x + 16, y + 14, WIDTH - 32, 26, m_sTitle, 20);
		float fy = y + 52;
		float fx = x + 24 + LABEL;
		float fw = FIELD;
		foreach (int i, ROOT_AE3_GmField f : m_aFields)
		{
			if (!Visible(i))
				continue;

			float rh = RowHeight(f);
			ROOT_AE3_UI.Text(m_wRoot, x + 16, fy + 5, LABEL, Math.Max(rh, ROW * 2) - 5, f.m_sLabel, 14, ROOT_AE3_UI.Rgb(0.75, 0.75, 0.78), false, true);
			BuildField(i, f, fx, fy, fw, rh, accent);
			fy += rh + GAP;
		}

		ROOT_AE3_UI.Button(m_wRoot, x + WIDTH - 236, y + h - 44, 104, 30, m_sOk, ID_OK, this, accent);
		ROOT_AE3_UI.Button(m_wRoot, x + WIDTH - 124, y + h - 44, 104, 30, "Cancel", ID_CANCEL, this);

		if (focus && !m_aEditOrder.IsEmpty())
		{
			EditBoxWidget first = EditBoxWidget.Cast(m_mEdits.Get(m_aEditOrder[0]));
			if (first)
			{
				GetGame().GetWorkspace().SetFocusedWidget(first);
				first.ActivateWriteMode();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildField(int i, ROOT_AE3_GmField f, float fx, float fy, float fw, float rh, Color accent)
	{
		int base = ID_FIELD + i * 100;
		Color on = accent;
		switch (f.m_eKind)
		{
			case ROOT_AE3_EGmField.TEXT:
			case ROOT_AE3_EGmField.PASSWORD:
			{
				ROOT_AE3_UI.Rect(m_wRoot, fx, fy, fw, rh, ROOT_AE3_UI.Rgb(0.05, 0.05, 0.06));
				EditBoxWidget e = ROOT_AE3_UI.Edit(m_wRoot, fx + 4, fy + 3, fw - 8, rh - 6, f.m_sValue, ID_EDIT + i, this, f.m_eKind == ROOT_AE3_EGmField.PASSWORD);
				m_mEdits.Set(i, e);
				m_aEditOrder.Insert(i);
				return;
			}

			case ROOT_AE3_EGmField.MULTILINE:
			{
				ROOT_AE3_UI.Rect(m_wRoot, fx, fy, fw, rh, ROOT_AE3_UI.Rgb(0.05, 0.05, 0.06));
				MultilineEditBoxWidget m = ROOT_AE3_UI.MultiEdit(m_wRoot, fx + 4, fy + 3, fw - 8, rh - 6, f.m_sValue, ID_EDIT + i, this);
				m_mEdits.Set(i, m);
				return;
			}

			case ROOT_AE3_EGmField.TOGGLE:
			{
				bool isOn = f.m_sValue == "1";
				ROOT_AE3_UI.Button(m_wRoot, fx, fy, 70, rh, "On", base + PART_OPTION + 1, this, Pick(isOn, on));
				ROOT_AE3_UI.Button(m_wRoot, fx + 74, fy, 70, rh, "Off", base + PART_OPTION, this, Pick(!isOn, on));
				return;
			}

			case ROOT_AE3_EGmField.CHOICE:
			{
				int n = f.m_aValues.Count();
				if (n == 0)
				{
					ROOT_AE3_UI.Text(m_wRoot, fx, fy + 5, fw, rh - 5, "-", 14);
					return;
				}

				int cols = Columns(f);
				if (cols > 0)
				{
					float bw = (fw - (cols - 1) * 4) / cols;
					for (int k = 0; k < n; k++)
					{
						int col = k % cols;
						int row = k / cols;
						ROOT_AE3_UI.Button(m_wRoot, fx + col * (bw + 4), fy + row * (ROW + 4), bw, ROW, f.m_aLabels[k], base + PART_OPTION + k, this, Pick(f.m_aValues[k] == f.m_sValue, on), null, 14);
					}

					return;
				}

				int cur = Math.Max(0, f.m_aValues.Find(f.m_sValue));
				ROOT_AE3_UI.Button(m_wRoot, fx, fy, 36, rh, "<", base + PART_PREV, this);
				ROOT_AE3_UI.Rect(m_wRoot, fx + 40, fy, fw - 80, rh, ROOT_AE3_UI.Rgb(0.05, 0.05, 0.06));
				ROOT_AE3_UI.Text(m_wRoot, fx + 48, fy + 5, fw - 96, rh - 5, string.Format("%1  (%2/%3)", f.m_aLabels[cur], cur + 1, n), 14);
				ROOT_AE3_UI.Button(m_wRoot, fx + fw - 36, fy, 36, rh, ">", base + PART_NEXT, this);
				return;
			}

			case ROOT_AE3_EGmField.PERMS:
			{
				array<string> names = {"r", "w", "x"};
				string p = Perms(f.m_sValue);
				ROOT_AE3_UI.Text(m_wRoot, fx, fy + 5, 70, rh - 5, "Owner", 14);
				ROOT_AE3_UI.Text(m_wRoot, fx + 220, fy + 5, 90, rh - 5, "Everyone", 14);
				for (int k = 0; k < 6; k++)
				{
					int col = k % 3;
					float bx = fx + 70 + col * 44;
					if (k >= 3)
						bx = fx + 310 + col * 44;

					ROOT_AE3_UI.Button(m_wRoot, bx, fy, 40, rh, names[col], base + PART_PERM + k, this, Pick(p.Get(k) != "-", on));
				}

				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static Color Pick(bool selected, Color on)
	{
		if (selected)
			return on;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Six character permission string ("rw-r--"), padded with "-".
	protected static string Perms(string value)
	{
		string p = value;
		while (p.Length() < 6)
		{
			p += "-";
		}

		return p.Substring(0, 6);
	}

	//------------------------------------------------------------------------------------------------
	protected void Submit()
	{
		ReadEdits();
		array<string> values = {};
		foreach (ROOT_AE3_GmField f : m_aFields)
		{
			values.Insert(f.m_sValue);
		}

		m_Dialog.Submit(values);
	}

	//------------------------------------------------------------------------------------------------
	//! Field button: toggle, choice option, choice arrows, permission bit. The form is rebuilt next frame.
	protected bool OnFieldClick(int id)
	{
		int i = (id - ID_FIELD) / 100;
		int part = (id - ID_FIELD) % 100;
		if (i < 0 || i >= m_aFields.Count())
			return false;

		ReadEdits();
		ROOT_AE3_GmField f = m_aFields[i];
		if (f.m_eKind == ROOT_AE3_EGmField.TOGGLE)
		{
			f.m_sValue = ROOT_AE3_Util.BoolStr(part == PART_OPTION + 1);
		}
		else if (f.m_eKind == ROOT_AE3_EGmField.CHOICE && !f.m_aValues.IsEmpty())
		{
			int n = f.m_aValues.Count();
			int cur = Math.Max(0, f.m_aValues.Find(f.m_sValue));
			if (part == PART_PREV)
				cur = (cur + n - 1) % n;
			else if (part == PART_NEXT)
				cur = (cur + 1) % n;
			else if (part >= PART_OPTION && part - PART_OPTION < n)
				cur = part - PART_OPTION;

			f.m_sValue = f.m_aValues[cur];
		}
		else if (f.m_eKind == ROOT_AE3_EGmField.PERMS && part >= PART_PERM && part < PART_PERM + 6)
		{
			int k = part - PART_PERM;
			array<string> names = {"r", "w", "x"};
			string p = Perms(f.m_sValue);
			string c = names[k % 3];
			if (p.Get(k) != "-")
				c = "-";

			f.m_sValue = p.Substring(0, k) + c + p.Substring(k + 1, 5 - k);
		}

		// Deferred: rebuilding removes the clicked widget.
		GetGame().GetCallqueue().CallLater(Rebuild, 0, false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case ID_OK: Submit(); return true;
			case ID_CANCEL: m_Dialog.Close(); return true;
		}

		if (id >= ID_FIELD)
			return OnFieldClick(id);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Enter in a single-line field: next single-line field, after the last one submit.
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (!finished || id < ID_EDIT || id >= ID_FIELD || !EditBoxWidget.Cast(w) || GetGame().GetInputManager().GetActionValue("MouseLeft") > 0)
			return false;

		int pos = m_aEditOrder.Find(id - ID_EDIT);
		if (pos >= 0 && pos + 1 < m_aEditOrder.Count())
		{
			EditBoxWidget next = EditBoxWidget.Cast(m_mEdits.Get(m_aEditOrder[pos + 1]));
			if (next)
			{
				GetGame().GetWorkspace().SetFocusedWidget(next);
				next.ActivateWriteMode();
				return true;
			}
		}

		Submit();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void ~ROOT_AE3_GmDialogForm()
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(Rebuild);
	}
}
