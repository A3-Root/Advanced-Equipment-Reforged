//! File manager: browse, open, new file / folder, copy, cut, paste, rename, delete to trash,
//! properties (owner, permissions), symlink, search, unlock.
class ROOT_AE3_AppFiles : ROOT_AE3_App
{
	protected static const int ID_UP = 1;
	protected static const int ID_REFRESH = 2;
	protected static const int ID_NEWDIR = 3;
	protected static const int ID_NEWFILE = 4;
	protected static const int ID_COPY = 5;
	protected static const int ID_CUT = 6;
	protected static const int ID_PASTE = 7;
	protected static const int ID_RENAME = 8;
	protected static const int ID_DELETE = 9;
	protected static const int ID_PROPS = 10;
	protected static const int ID_LINK = 11;
	protected static const int ID_SEARCH = 12;
	protected static const int ID_PATH = 13;
	protected static const int ID_NAME = 14;
	protected static const int ID_APPLY = 15;
	protected static const int ID_OPEN = 16;
	protected static const int ID_HOME = 17;
	protected static const int LIST = 1;

	protected string m_sPath = "~";
	protected ROOT_AE3_ListView m_List;
	protected EditBoxWidget m_wPath;
	protected EditBoxWidget m_wName;
	protected TextWidget m_wInfo;
	protected string m_sMode;		//!< pending action for the name field: newdir, newfile, rename, link, search, chmod, chown, unlock
	protected static string s_sClip;
	protected static bool s_bCut;
	protected ref array<ref array<string>> m_aEntries = {};

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Files";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "listView";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(780, 520, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		if (!m_sArg.IsEmpty())
		{
			m_sPath = m_sArg;
			m_sArg = "";
		}

		float w = Width();
		float h = Height();
		array<string> labels = {"Up", "Home", "Refresh", "Open", "New folder", "New file", "Copy", "Cut", "Paste", "Rename", "Delete", "Properties", "Link", "Search"};
		array<int> ids = {ID_UP, ID_HOME, ID_REFRESH, ID_OPEN, ID_NEWDIR, ID_NEWFILE, ID_COPY, ID_CUT, ID_PASTE, ID_RENAME, ID_DELETE, ID_PROPS, ID_LINK, ID_SEARCH};
		float x = 6;
		float y = 6;
		foreach (int i, string l : labels)
		{
			float bw = 16 + l.Length() * 7;
			if (x + bw > w - 6)
			{
				x = 6;
				y += 30;
			}

			Btn(x, y, bw, 26, l, ids[i]);
			x += bw + 4;
		}

		y += 32;
		m_wPath = Field(6, y, w - 12, m_sPath, ID_PATH);
		y += 32;
		m_List = List(LIST, 6, y, w - 12, h - y - 70);
		array<string> headers = {"Name", "Type", "Size", "Owner", "Permissions"};
		array<float> widths = {(w - 12) * 0.42, (w - 12) * 0.12, (w - 12) * 0.14, (w - 12) * 0.14, (w - 12) * 0.18};
		m_List.SetColumns(headers, widths);
		m_wName = Field(6, h - 62, w - 120, "", ID_NAME);
		Btn(w - 108, h - 62, 102, 26, "Apply", ID_APPLY);
		m_wInfo = Label(6, h - 30, w - 12, 22, "", 13);
		Load();
	}

	//------------------------------------------------------------------------------------------------
	protected void Load()
	{
		Request1("fs_list", m_sPath);
	}

	//------------------------------------------------------------------------------------------------
	protected string SelectedPath()
	{
		array<string> r = m_List.Selected();
		if (!r)
			return "";

		int i = m_List.m_iSelected;
		if (i < 0 || i >= m_aEntries.Count())
			return "";

		return m_aEntries[i][0];
	}

	//------------------------------------------------------------------------------------------------
	protected void Prompt(string mode, string hint, string text = "")
	{
		m_sMode = mode;
		ROOT_AE3_UI.SetEdit(m_wName, text);
		m_wInfo.SetText(hint);
		GetGame().GetWorkspace().SetFocusedWidget(m_wName);
		m_wName.ActivateWriteMode();
	}

	//------------------------------------------------------------------------------------------------
	protected string Join(string dir, string name)
	{
		if (dir.EndsWith("/"))
			return dir + name;

		return dir + "/" + name;
	}

	//------------------------------------------------------------------------------------------------
	protected void Apply()
	{
		string v = m_wName.GetText().Trim();
		string sel = SelectedPath();
		switch (m_sMode)
		{
			case "newdir": Request1("fs_mkdir", Join(m_sPath, v)); break;
			case "newfile": Request2("fs_save", Join(m_sPath, v), ""); break;
			case "rename": Request2("fs_move", sel, Join(m_sPath, v)); break;
			case "link": Request2("fs_symlink", sel, Join(m_sPath, v)); break;
			case "search": Request2("fs_search", m_sPath, v); break;
			case "chmod": Request2("fs_chmod", sel, v); break;
			case "chown": Request2("fs_chown", sel, v); break;
			case "unlock":
			{
				array<string> args = {sel, v, "1"};
				Request("fs_unlock", args);
				break;
			}
		}

		m_sMode = "";
		ROOT_AE3_UI.SetEdit(m_wName, "");
	}

	//------------------------------------------------------------------------------------------------
	//! Opens a file in the fitting app.
	protected void OpenEntry(array<string> e)
	{
		if (e[1] == "dir")
		{
			m_sPath = e[0];
			ROOT_AE3_UI.SetEdit(m_wPath, m_sPath);
			Load();
			return;
		}

		if (e[5] == "1")
		{
			m_Desktop.OpenApp(ROOT_AE3_AppMedia, e[0]);
			return;
		}

		if (e[6] == "1")
		{
			Prompt("unlock", "Password protected. Enter the password and press Apply to remove the protection.");
			return;
		}

		if (e[7] == "1")
		{
			Request1("run_file", e[0]);
			return;
		}

		m_Desktop.OpenApp(ROOT_AE3_AppNotepad, e[0]);
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
			case "fs_list":
			{
				m_sPath = Arg(payload, 0);
				ROOT_AE3_UI.SetEdit(m_wPath, m_sPath);
				m_List.Clear();
				m_aEntries.Clear();
				for (int i = 1; i < payload.Count(); i++)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(payload[i]);
					if (e.Count() < 8)
						continue;

					m_aEntries.Insert(e);
					string type = "file";
					if (e[1] == "dir")
						type = "folder";
					else if (e[1] == "link")
						type = "link";
					else if (e[5] == "1")
						type = "image";
					else if (e[6] == "1")
						type = "locked";
					else if (e[7] == "1")
						type = "program";

					array<string> row = {ROOT_AE3_Filesystem.BaseName(e[0]), type, e[2], e[3], e[4]};
					m_List.AddRow(row);
				}

				m_List.Render();
				Status(string.Format("%1 items", m_aEntries.Count()));
				return;
			}

			case "fs_search":
			{
				m_List.Clear();
				m_aEntries.Clear();
				foreach (string p : payload)
				{
					array<string> e = {p, "file", "", "", "", "0", "0", "0"};
					m_aEntries.Insert(e);
					array<string> row = {p, "", "", "", ""};
					m_List.AddRow(row);
				}

				m_List.Render();
				Status(string.Format("%1 results", payload.Count()));
				return;
			}

			case "fs_stat":
			{
				m_wInfo.SetText(Arg(payload, 0));
				return;
			}

			case "fs_copy":
			case "fs_move":
			case "fs_mkdir":
			case "fs_save":
			case "fs_delete":
			case "fs_symlink":
			case "fs_chmod":
			case "fs_chown":
			case "fs_unlock":
			{
				Status(Arg(payload, 0));
				Load();
				return;
			}
		}

		Status(Arg(payload, 0));
	}

	//------------------------------------------------------------------------------------------------
	override void OnListActivate(int listId, int row)
	{
		if (row >= 0 && row < m_aEntries.Count())
			OpenEntry(m_aEntries[row]);
	}

	//------------------------------------------------------------------------------------------------
	override void OnListSelect(int listId, int row)
	{
		string p = SelectedPath();
		if (!p.IsEmpty())
			Request1("fs_stat", p);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (!finished)
			return false;

		if (id == ID_PATH)
		{
			m_sPath = m_wPath.GetText();
			Load();
			return true;
		}

		if (id == ID_NAME)
		{
			Apply();
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		string sel = SelectedPath();
		switch (id)
		{
			case ID_UP:
			{
				m_sPath = m_sPath + "/..";
				Load();
				return true;
			}

			case ID_HOME:
			{
				m_sPath = "~";
				Load();
				return true;
			}

			case ID_REFRESH: Load(); return true;
			case ID_OPEN:
			{
				if (m_List.m_iSelected >= 0 && m_List.m_iSelected < m_aEntries.Count())
					OpenEntry(m_aEntries[m_List.m_iSelected]);

				return true;
			}

			case ID_NEWDIR: Prompt("newdir", "Name of the new folder, then Apply."); return true;
			case ID_NEWFILE: Prompt("newfile", "Name of the new file, then Apply."); return true;
			case ID_COPY:
			case ID_CUT:
			{
				if (sel.IsEmpty())
					return true;

				s_sClip = sel;
				s_bCut = id == ID_CUT;
				Status("Clipboard: " + sel);
				return true;
			}

			case ID_PASTE:
			{
				if (s_sClip.IsEmpty())
					return true;

				string dst = Join(m_sPath, ROOT_AE3_Filesystem.BaseName(s_sClip));
				if (s_bCut)
				{
					Request2("fs_move", s_sClip, dst);
					s_sClip = "";
				}
				else
				{
					Request2("fs_copy", s_sClip, dst);
				}

				Status("Pasting...");
				return true;
			}

			case ID_RENAME:
			{
				if (!sel.IsEmpty())
					Prompt("rename", "New name, then Apply.", ROOT_AE3_Filesystem.BaseName(sel));

				return true;
			}

			case ID_DELETE:
			{
				if (!sel.IsEmpty())
					Request1("fs_delete", sel);

				return true;
			}

			case ID_PROPS:
			{
				if (sel.IsEmpty())
					return true;

				if (m_sMode == "chmod")
					Prompt("chown", "New owner, then Apply.");
				else
					Prompt("chmod", "Permissions as rwxr-x (owner then everyone), then Apply. Press Properties again for the owner.");

				return true;
			}

			case ID_LINK:
			{
				if (!sel.IsEmpty())
					Prompt("link", "Name of the link to the selected item, then Apply.", ROOT_AE3_Filesystem.BaseName(sel) + "_link");

				return true;
			}

			case ID_SEARCH: Prompt("search", "Search pattern (regular expression), then Apply."); return true;
			case ID_APPLY: Apply(); return true;
		}

		return false;
	}
}
