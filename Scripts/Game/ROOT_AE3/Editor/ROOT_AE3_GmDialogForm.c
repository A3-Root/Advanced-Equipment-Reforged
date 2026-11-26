//! Widgets and events of ROOT_AE3_GmDialog.
class ROOT_AE3_GmDialogForm : ROOT_AE3_UiListener
{
	protected ROOT_AE3_GmDialog m_Dialog;
	protected ref array<EditBoxWidget> m_aFields = {};

	//------------------------------------------------------------------------------------------------
	//! extra: label of a third button (id 3, ROOT_AE3_GmDialog.Extra), empty = none.
	void ROOT_AE3_GmDialogForm(ROOT_AE3_GmDialog dialog, Widget root, string title, array<string> labels, array<string> defaults, string okLabel = "OK", string extra = "")
	{
		m_Dialog = dialog;
		float sw, sh;
		ROOT_AE3_UI.ScreenSize(sw, sh);
		float w = 640;
		float h = 110 + labels.Count() * 36;
		float x = (sw - w) * 0.5;
		float y = (sh - h) * 0.5;
		ROOT_AE3_UI.Rect(root, 0, 0, sw, sh, ROOT_AE3_UI.Rgb(0, 0, 0, 0.4));
		ROOT_AE3_UI.Rect(root, x, y, w, h, ROOT_AE3_UI.Rgb(0.12, 0.13, 0.15, 0.97));
		ROOT_AE3_UI.Rect(root, x, y, w, 4, ROOT_AE3_UI.Rgb(0.91, 0.33, 0.13));
		ROOT_AE3_UI.Text(root, x + 16, y + 14, w - 32, 26, title, 20);
		foreach (int i, string l : labels)
		{
			float fy = y + 52 + i * 36;
			ROOT_AE3_UI.Text(root, x + 16, fy + 4, 250, 22, l, 14, ROOT_AE3_UI.Rgb(0.75, 0.75, 0.78));
			ROOT_AE3_UI.Rect(root, x + 270, fy, w - 286, 28, ROOT_AE3_UI.Rgb(0.05, 0.05, 0.06));
			string def;
			if (i < defaults.Count())
				def = defaults[i];

			EditBoxWidget e = ROOT_AE3_UI.Edit(root, x + 274, fy + 2, w - 294, 24, def, 100 + i, this);
			m_aFields.Insert(e);
		}

		ROOT_AE3_UI.Button(root, x + w - 236, y + h - 44, 104, 30, okLabel, 1, this, ROOT_AE3_UI.Rgb(0.91, 0.33, 0.13));
		ROOT_AE3_UI.Button(root, x + w - 124, y + h - 44, 104, 30, "Cancel", 2, this);
		if (!extra.IsEmpty())
		{
			ROOT_AE3_UI.Button(root, x + 16, y + h - 44, 140, 30, extra, 3, this, ROOT_AE3_UI.Rgb(0.2, 0.45, 0.25));
			return;
		}

		if (!m_aFields.IsEmpty() && m_aFields[0])
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_aFields[0]);
			m_aFields[0].ActivateWriteMode();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Submit()
	{
		array<string> values = {};
		foreach (EditBoxWidget e : m_aFields)
		{
			if (e)
				values.Insert(e.GetText());
			else
				values.Insert("");
		}

		m_Dialog.Submit(values);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == 1)
		{
			Submit();
			return true;
		}

		if (id == 2)
		{
			m_Dialog.Close();
			return true;
		}

		if (id == 3)
		{
			m_Dialog.Extra();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (!finished || id < 100 || GetGame().GetInputManager().GetActionValue("MouseLeft") > 0)
			return false;

		int next = id - 100 + 1;
		if (next < m_aFields.Count() && m_aFields[next])
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_aFields[next]);
			m_aFields[next].ActivateWriteMode();
			return true;
		}

		Submit();
		return true;
	}
}
