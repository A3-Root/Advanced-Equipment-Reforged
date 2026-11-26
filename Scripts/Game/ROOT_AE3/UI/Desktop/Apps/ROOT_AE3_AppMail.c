//! Mail client: inbox, sent, read, compose, delete, addresses.
class ROOT_AE3_AppMail : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Mail";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "mail";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(820, 540, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_ListView m_List;
	protected ref array<string> m_aFiles = {};
	protected string m_sFolder = "inbox";
	protected TextWidget m_wBody;
	protected EditBoxWidget m_wTo;
	protected EditBoxWidget m_wSubject;
	protected MultilineEditBoxWidget m_wText;
	protected EditBoxWidget m_wFrom;
	protected bool m_bCompose;
	protected string m_sMyAddress;

	override void Build()
	{
		float w = Width();
		float h = Height();
		Btn(6, 6, 80, 26, "Inbox", 1);
		Btn(90, 6, 80, 26, "Sent", 2);
		Btn(174, 6, 90, 26, "Compose", 3);
		Btn(268, 6, 80, 26, "Delete", 4);
		Btn(352, 6, 80, 26, "Refresh", 5);
		if (m_bCompose)
		{
			Label(10, 44, 70, 22, "From");
			m_wFrom = Field(80, 40, w - 90, m_sMyAddress, 20);
			Label(10, 76, 70, 22, "To");
			m_wTo = Field(80, 72, w - 90, "", 21);
			Label(10, 108, 70, 22, "Subject");
			m_wSubject = Field(80, 104, w - 90, "", 22);
			ROOT_AE3_UI.Rect(GetContent(), 6, 136, w - 12, h - 176, ROOT_AE3_Theme.Field());
			m_wText = ROOT_AE3_UI.MultiEdit(GetContent(), 10, 140, w - 20, h - 184, "", 23, this);
			if (m_wText)
				m_wText.SetColor(ROOT_AE3_Theme.Text());

			Btn(6, h - 34, 100, 28, "Send", 6);
			Btn(112, h - 34, 100, 28, "Cancel", 7);
			Request0("addr_list");
			return;
		}

		m_List = List(1, 6, 38, w * 0.45, h - 44);
		array<string> headers = {"From / To", "Subject", "Date"};
		array<float> widths = {w * 0.45 * 0.35, w * 0.45 * 0.4, w * 0.45 * 0.25};
		m_List.SetColumns(headers, widths);
		ROOT_AE3_UI.Rect(GetContent(), w * 0.45 + 12, 38, w * 0.55 - 18, h - 44, ROOT_AE3_Theme.Field());
		m_wBody = Label(w * 0.45 + 20, 44, w * 0.55 - 34, h - 56, "", 15, false, true);
		Request1("mail_list", m_sFolder);
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
			case "mail_list":
			{
				m_List.Clear();
				m_aFiles.Clear();
				foreach (string p : payload)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(p);
					if (e.Count() < 4)
						continue;

					m_aFiles.Insert(e[0]);
					array<string> row = {e[1], e[2], e[3]};
					m_List.AddRow(row);
				}

				m_List.Render();
				Status(string.Format("%1: %2 mails", m_sFolder, m_aFiles.Count()));
				return;
			}

			case "mail_read":
			{
				if (m_wBody)
					m_wBody.SetText(Arg(payload, 0));

				return;
			}

			case "addr_list":
			{
				m_sMyAddress = Arg(payload, 0);
				if (m_wFrom && m_wFrom.GetText().IsEmpty())
					ROOT_AE3_UI.SetEdit(m_wFrom, m_sMyAddress);

				return;
			}

			case "mail_send":
			{
				m_bCompose = false;
				Redraw();
				Status("Mail sent.");
				return;
			}
		}

		Status(Arg(payload, 0));
		if (!m_bCompose)
			Request1("mail_list", m_sFolder);
	}

	//------------------------------------------------------------------------------------------------
	override void OnListSelect(int listId, int row)
	{
		if (row >= 0 && row < m_aFiles.Count())
			Request1("mail_read", m_aFiles[row]);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		switch (id)
		{
			case 1:
			case 2:
			{
				m_sFolder = "inbox";
				if (id == 2)
					m_sFolder = "sent";

				m_bCompose = false;
				Redraw();
				return true;
			}

			case 3: m_bCompose = true; Redraw(); return true;
			case 4:
			{
				if (m_List && m_List.m_iSelected >= 0 && m_List.m_iSelected < m_aFiles.Count())
					Request1("mail_delete", m_aFiles[m_List.m_iSelected]);

				return true;
			}

			case 5: Redraw(); return true;
			case 6:
			{
				array<string> args = {m_wFrom.GetText(), m_wTo.GetText(), m_wSubject.GetText(), m_wText.GetText()};
				Request("mail_send", args);
				return true;
			}

			case 7: m_bCompose = false; Redraw(); return true;
		}

		return false;
	}
}
