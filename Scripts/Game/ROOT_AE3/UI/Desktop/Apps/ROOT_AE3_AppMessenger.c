//! Chat between computers by handle.
class ROOT_AE3_AppMessenger : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Messenger";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "comments";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(720, 480, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_ListView m_List;
	protected ref array<string> m_aHandles = {};
	protected string m_sPeer;
	protected TextWidget m_wLog;
	protected EditBoxWidget m_wMsg;
	protected EditBoxWidget m_wHandle;
	protected float m_fTimer;

	override void Build()
	{
		float w = Width();
		float h = Height();
		Label(8, 10, 90, 22, "My handle");
		m_wHandle = Field(96, 6, 180, "", 1);
		Btn(282, 6, 70, 26, "Set", 2);
		m_List = List(1, 6, 40, 200, h - 46);
		array<string> headers = {"Contacts"};
		array<float> widths = {200};
		m_List.SetColumns(headers, widths);
		ROOT_AE3_UI.Rect(GetContent(), 212, 40, w - 218, h - 80, ROOT_AE3_Theme.Field());
		m_wLog = Label(220, 46, w - 234, h - 92, "Select a contact.", 14, false, true);
		m_wMsg = Field(212, h - 34, w - 300, "", 3);
		Btn(w - 82, h - 34, 76, 26, "Send", 4);
		Request0("chat_handles");
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float dt)
	{
		m_fTimer += dt;
		if (m_fTimer > 3 && !m_sPeer.IsEmpty())
		{
			m_fTimer = 0;
			Request1("chat_read", m_sPeer);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			return;
		}

		switch (cmd)
		{
			case "chat_handles":
			{
				ROOT_AE3_UI.SetEdit(m_wHandle, Arg(payload, 0));
				m_List.Clear();
				m_aHandles.Clear();
				for (int i = 1; i < payload.Count(); i++)
				{
					m_aHandles.Insert(payload[i]);
					array<string> row = {payload[i]};
					m_List.AddRow(row);
				}

				m_List.Render();
				return;
			}

			case "chat_read":
			{
				string log = Arg(payload, 0);
				array<string> lines = {};
				log.Split("\n", lines, true);
				int start = Math.Max(0, lines.Count() - 18);
				string text;
				for (int i = start; i < lines.Count(); i++)
				{
					text += lines[i] + "\n";
				}

				m_wLog.SetText(text);
				return;
			}

			case "chat_send":
			{
				Request1("chat_read", m_sPeer);
				return;
			}
		}

		Status(Arg(payload, 0));
		Request0("chat_handles");
	}

	//------------------------------------------------------------------------------------------------
	override void OnListSelect(int listId, int row)
	{
		if (row < 0 || row >= m_aHandles.Count())
			return;

		m_sPeer = m_aHandles[row];
		Request1("chat_read", m_sPeer);
		Status("Chat with " + m_sPeer);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendMsg()
	{
		if (m_sPeer.IsEmpty() || m_wMsg.GetText().IsEmpty())
			return;

		Request2("chat_send", m_sPeer, m_wMsg.GetText());
		ROOT_AE3_UI.SetEdit(m_wMsg, "");
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (finished && id == 3)
		{
			SendMsg();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == 2)
		{
			Request1("chat_set_handle", m_wHandle.GetText());
			return true;
		}

		if (id == 4)
		{
			SendMsg();
			return true;
		}

		return false;
	}
}
