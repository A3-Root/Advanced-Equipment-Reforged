//! Calendar: events by month, add and remove.
class ROOT_AE3_AppCalendar : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Calendar";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "time";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(720, 480, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_ListView m_List;
	protected ref array<ref array<string>> m_aEvents = {};
	protected EditBoxWidget m_wDate;
	protected EditBoxWidget m_wTime;
	protected EditBoxWidget m_wTitle;
	protected EditBoxWidget m_wText;

	override void Build()
	{
		float w = Width();
		float h = Height();
		m_List = List(1, 6, 6, w - 12, h - 120);
		array<string> headers = {"Date", "Time", "Title", "Description"};
		array<float> widths = {(w - 12) * 0.17, (w - 12) * 0.1, (w - 12) * 0.28, (w - 12) * 0.45};
		m_List.SetColumns(headers, widths);
		float y = h - 106;
		Label(8, y + 4, 40, 22, "Date");
		m_wDate = Field(50, y, 120, ROOT_AE3_Util.Today(), 1);
		Label(178, y + 4, 40, 22, "Time");
		m_wTime = Field(220, y, 80, "12:00", 2);
		Label(308, y + 4, 40, 22, "Title");
		m_wTitle = Field(350, y, w - 356, "", 3);
		Label(8, y + 38, 80, 22, "Description");
		m_wText = Field(96, y + 34, w - 102, "", 4);
		Btn(6, h - 34, 100, 28, "Add", 5);
		Btn(112, h - 34, 140, 28, "Remove selected", 6);
		Btn(258, h - 34, 100, 28, "Refresh", 7);
		Request0("cal_list");
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		if (cmd != "cal_list")
		{
			Request0("cal_list");
			return;
		}

		m_List.Clear();
		m_aEvents.Clear();
		foreach (string p : payload)
		{
			array<string> e = ROOT_AE3_Codec.SplitSub(p);
			if (e.Count() < 4)
				continue;

			m_aEvents.Insert(e);
			m_List.AddRow(e);
		}

		m_List.Render();
		Status(string.Format("%1 events", m_aEvents.Count()));
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == 5)
		{
			array<string> args = {m_wDate.GetText(), m_wTime.GetText(), m_wTitle.GetText(), m_wText.GetText()};
			Request("cal_add", args);
			return true;
		}

		if (id == 6)
		{
			int i = m_List.m_iSelected;
			if (i >= 0 && i < m_aEvents.Count())
				Request2("cal_remove", m_aEvents[i][0], m_aEvents[i][2]);

			return true;
		}

		if (id == 7)
		{
			Request0("cal_list");
			return true;
		}

		return false;
	}
}
