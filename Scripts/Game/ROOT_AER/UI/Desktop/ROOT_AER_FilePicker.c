//! Modal Open / Save dialog of the desktop: places on the left, folder content, file name field.
//! Click selects, double click opens a folder or picks a file, Up goes to the parent folder.
//! The chosen absolute path goes to target.OnDialog(tag, true, path); Cancel sends ok false.
class ROOT_AER_FilePicker : ROOT_AER_GuiReplyHandler
{
	protected static const int ID_BG = 1;
	protected static const int ID_UP = 2;
	protected static const int ID_OK = 3;
	protected static const int ID_CANCEL = 4;
	protected static const int ID_NAME = 5;
	protected static const int ID_LIST = 6;
	protected static const int ID_NEWDIR = 7;
	protected static const int ID_PLACE = 50;
	protected static const int ID_ROW = 1000;
	protected static const float W = 680;
	protected static const float H = 460;
	protected static const float ROW_H = 28;
	protected static const float NAV_W = 150;

	protected ROOT_AER_DesktopView m_Desktop;
	protected ROOT_AER_UiListener m_Target;
	protected string m_sTag;
	protected string m_sTitle;
	protected bool m_bSave;
	protected bool m_bFolders;
	protected string m_sOk;
	protected string m_sDir;
	protected ref array<ref array<string>> m_aEntries = {};
	protected int m_iSel = -1;
	protected int m_iScroll;
	protected FrameWidget m_wRoot;
	protected FrameWidget m_wList;
	protected TextWidget m_wPath;
	protected EditBoxWidget m_wName;
	protected ref ROOT_AER_ForwardListener m_ListListener;
	protected float m_fX;
	protected float m_fY;
	protected float m_fListW;
	protected float m_fListH;

	//------------------------------------------------------------------------------------------------
	//! save: Save dialog (name may be new). folders: pick a folder instead of a file.
	void ROOT_AER_FilePicker(ROOT_AER_DesktopView desktop, Widget parent, float areaW, float areaH, ROOT_AER_UiListener target, string tag, string title, string start, bool save, string fileName, string okLabel, bool folders)
	{
		m_Desktop = desktop;
		m_Target = target;
		m_sTag = tag;
		m_sTitle = title;
		m_bSave = save;
		m_bFolders = folders;
		m_sOk = okLabel;
		m_fX = Math.Max(0, (areaW - W) * 0.5);
		m_fY = Math.Max(0, (areaH - H) * 0.45);
		m_wRoot = ROOT_AER_UI.Frame(parent, 0, 0, areaW, areaH, false);
		m_wRoot.SetZOrder(140000);
		Build(areaW, areaH, fileName);
		m_sDir = start;
		if (m_sDir.IsEmpty())
			m_sDir = ROOT_AER_Filesystem.HomeOf(ROOT_AER_Client.Get().m_sUser);

		Load(m_sDir);
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		ROOT_AER_Client.Get().Forget(this);
		ReleaseHandlers();
		m_ListListener = null;
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();

		m_wRoot = null;
	}

	//------------------------------------------------------------------------------------------------
	bool IsTyping()
	{
		return m_wName && m_wName.IsInWriteMode();
	}

	//------------------------------------------------------------------------------------------------
	protected void Build(float areaW, float areaH, string fileName)
	{
		float x = m_fX;
		float y = m_fY;
		ROOT_AER_UI.Rect(m_wRoot, 0, 0, areaW, areaH, ROOT_AER_UI.Rgb(0, 0, 0, 0.3), ID_BG, this);
		ROOT_AER_UI.Shadow(m_wRoot, x, y, W, H, 16);
		ROOT_AER_UI.Box(m_wRoot, x, y, W, H, ROOT_AER_Theme.Panel());
		ROOT_AER_UI.Rect(m_wRoot, x + 1, y + 1, W - 2, 40, ROOT_AER_Theme.Title());
		ROOT_AER_UI.Text(m_wRoot, x + 16, y + 11, W - 32, 20, m_sTitle, 16, ROOT_AER_Theme.Text());

		// Address row.
		ROOT_AER_UI.Box(m_wRoot, x + 12, y + 50, 34, 30, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Border());
		ROOT_AER_UI.IconButton(m_wRoot, x + 13, y + 51, 32, 28, "g_up", ID_UP, this, 14, ROOT_AER_Theme.Text(), ROOT_AER_Theme.Button(), ROOT_AER_Theme.Hover());
		ROOT_AER_UI.Box(m_wRoot, x + 52, y + 50, W - 64 - 130, 30, ROOT_AER_Theme.Field(), ROOT_AER_Theme.Border());
		ROOT_AER_UI.Icon(m_wRoot, x + 60, y + 57, 16, "folder");
		m_wPath = ROOT_AER_UI.Text(m_wRoot, x + 82, y + 56, W - 64 - 160, 20, "", 14, ROOT_AER_Theme.Text());
		ROOT_AER_UI.Box(m_wRoot, x + W - 136, y + 50, 124, 30, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Border());
		ROOT_AER_UI.ItemButton(m_wRoot, x + W - 135, y + 51, 122, 28, "g_newfolder", "New folder", ID_NEWDIR, this, 13, ROOT_AER_Theme.Text(), ROOT_AER_Theme.Button(), ROOT_AER_Theme.Hover());

		// Places.
		float py = y + 90;
		ROOT_AER_UI.Rect(m_wRoot, x + 1, py, NAV_W, H - 90 - 64, ROOT_AER_Theme.Sidebar());
		array<string> labels = {"Home", "This computer", "Drives", "Temp"};
		array<string> icons = {"g_home", "computer", "drive", "folder"};
		foreach (int i, string l : labels)
		{
			ROOT_AER_UI.ItemButton(m_wRoot, x + 5, py + 6 + i * 32, NAV_W - 8, 30, icons[i], l, ID_PLACE + i, this, 14, ROOT_AER_Theme.Text(), ROOT_AER_UI.Rgb(0, 0, 0, 0), ROOT_AER_Theme.Hover());
		}

		m_fListW = W - NAV_W - 14;
		m_fListH = H - 90 - 64;
		m_wList = ROOT_AER_UI.Frame(m_wRoot, x + NAV_W + 6, py, m_fListW, m_fListH);

		// Name row and buttons.
		float by = y + H - 56;
		ROOT_AER_UI.Rect(m_wRoot, x + 1, by - 8, W - 2, 63, ROOT_AER_Theme.Bar());
		string nameLabel = "File name";
		if (m_bFolders)
			nameLabel = "Folder";

		ROOT_AER_UI.Text(m_wRoot, x + 16, by + 7, 80, 20, nameLabel, 14, ROOT_AER_Theme.Text());
		ROOT_AER_UI.Box(m_wRoot, x + 96, by + 2, W - 96 - 252, 30, ROOT_AER_Theme.Field(), ROOT_AER_Theme.Border());
		m_wName = ROOT_AER_UI.Edit(m_wRoot, x + 102, by + 4, W - 96 - 264, 26, fileName, ID_NAME, this);
		if (m_wName)
			m_wName.SetColor(ROOT_AER_Theme.Text());

		ROOT_AER_UI.Button(m_wRoot, x + W - 244, by + 2, 110, 30, m_sOk, ID_OK, this, ROOT_AER_Theme.Accent(), ROOT_AER_Theme.OnAccent(), 14);
		ROOT_AER_UI.Rect(m_wRoot, x + W - 126, by + 2, 112, 30, ROOT_AER_Theme.Border());
		ROOT_AER_UI.Button(m_wRoot, x + W - 125, by + 3, 110, 28, "Cancel", ID_CANCEL, this, ROOT_AER_Theme.Button(), ROOT_AER_Theme.Text(), 14);
	}

	//------------------------------------------------------------------------------------------------
	protected void Load(string dir)
	{
		array<string> args = {dir};
		ROOT_AER_Client.Get().Request("fs_list", args, this);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsDir(int i)
	{
		return i >= 0 && i < m_aEntries.Count() && m_aEntries[i][1] == "dir";
	}

	//------------------------------------------------------------------------------------------------
	protected string Join(string dir, string name)
	{
		if (name.StartsWith("/"))
			return name;

		if (dir == "/")
			return "/" + name;

		return dir + "/" + name;
	}

	//------------------------------------------------------------------------------------------------
	protected void RenderList()
	{
		if (!m_wList)
			return;

		m_ListListener = new ROOT_AER_ForwardListener(this);
		ROOT_AER_UI.Clear(m_wList);
		ROOT_AER_UI.Rect(m_wList, 0, 0, m_fListW, m_fListH, ROOT_AER_Theme.Field(), ID_LIST, m_ListListener);
		int lines = Math.Floor(m_fListH / ROW_H);
		m_iScroll = Math.ClampInt(m_iScroll, 0, Math.Max(0, m_aEntries.Count() - lines));
		float y = 0;
		for (int r = m_iScroll; r < Math.Min(m_aEntries.Count(), m_iScroll + lines); r++)
		{
			array<string> e = m_aEntries[r];
			Color bg = ROOT_AER_UI.Rgb(0, 0, 0, 0);
			if (r == m_iSel)
				bg = ROOT_AER_Theme.Select();

			Color fg = ROOT_AER_Theme.Text();
			if (m_bFolders && e[1] != "dir")
				fg = ROOT_AER_Theme.Muted();

			ROOT_AER_UI.Hit(m_wList, 2, y + 1, m_fListW - 4, ROW_H - 2, ID_ROW + r, m_ListListener, bg, ROOT_AER_Theme.Hover());
			ROOT_AER_UI.Icon(m_wList, 10, y + 5, 18, ROOT_AER_Icons.ForFile(e[1], e[5] == "1", e[6] == "1", e[7] == "1", ROOT_AER_Filesystem.BaseName(e[0])));
			ROOT_AER_UI.Text(m_wList, 36, y + 6, m_fListW - 140, ROW_H - 8, ROOT_AER_Filesystem.BaseName(e[0]), 14, fg);
			if (e[1] != "dir")
				ROOT_AER_UI.Text(m_wList, m_fListW - 100, y + 6, 90, ROW_H - 8, e[2], 13, ROOT_AER_Theme.Muted(), false, false, 2);

			y += ROW_H;
		}

		if (m_aEntries.IsEmpty())
			ROOT_AER_UI.Text(m_wList, 0, m_fListH * 0.4, m_fListW, 20, "This folder is empty.", 14, ROOT_AER_Theme.Muted(), false, false, 1);
	}

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (cmd == "fs_mkdir")
		{
			if (!ok && m_wPath)
				m_wPath.SetText(Arg(payload, 0));

			Load(m_sDir);
			return;
		}

		if (cmd != "fs_list")
			return;

		if (!ok)
		{
			if (m_wPath)
				m_wPath.SetText(m_sDir + "   (" + Arg(payload, 0) + ")");

			return;
		}

		m_sDir = Arg(payload, 0);
		m_aEntries.Clear();
		array<ref array<string>> files = {};
		for (int i = 1; i < payload.Count(); i++)
		{
			array<string> e = ROOT_AER_Codec.SplitSub(payload[i]);
			if (e.Count() < 8)
				continue;

			if (e[1] == "dir")
				m_aEntries.Insert(e);
			else
				files.Insert(e);
		}

		foreach (array<string> f : files)
		{
			m_aEntries.Insert(f);
		}

		m_iSel = -1;
		m_iScroll = 0;
		if (m_wPath)
			m_wPath.SetText(m_sDir);

		RenderList();
	}

	//------------------------------------------------------------------------------------------------
	protected static string Arg(array<string> p, int i)
	{
		if (!p || i < 0 || i >= p.Count())
			return "";

		return p[i];
	}

	//------------------------------------------------------------------------------------------------
	//! OK: a folder name opens the folder (file mode), otherwise the path is returned.
	void Accept()
	{
		string name;
		if (m_wName)
			name = m_wName.GetText().Trim();

		if (m_bFolders)
		{
			string pick = m_sDir;
			if (IsDir(m_iSel))
				pick = m_aEntries[m_iSel][0];
			else if (!name.IsEmpty())
				pick = Join(m_sDir, name);

			Finish(true, pick);
			return;
		}

		if (name.IsEmpty())
		{
			if (IsDir(m_iSel))
				Load(m_aEntries[m_iSel][0]);

			return;
		}

		string path = Join(m_sDir, name);
		foreach (array<string> e : m_aEntries)
		{
			if (e[0] == path && e[1] == "dir")
			{
				Load(path);
				ROOT_AER_UI.SetEdit(m_wName, "");
				return;
			}
		}

		Finish(true, path);
	}

	//------------------------------------------------------------------------------------------------
	void Cancel()
	{
		Finish(false, "");
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish(bool ok, string path)
	{
		ROOT_AER_UiListener target = m_Target;
		string tag = m_sTag;
		ROOT_AER_DesktopView desktop = m_Desktop;
		GetGame().GetWorkspace().SetFocusedWidget(null);
		if (desktop)
			desktop.ClosePicker();

		if (target)
			target.OnDialog(tag, ok, path);
	}

	//------------------------------------------------------------------------------------------------
	//! Arrow keys move the selection, Enter accepts, Backspace goes up.
	void OnKey(KeyCode key)
	{
		switch (key)
		{
			case KeyCode.KC_RETURN:
			case KeyCode.KC_NUMPADENTER:
			{
				if (IsDir(m_iSel) && (!m_wName || m_wName.GetText().Trim().IsEmpty()) && !m_bFolders)
					Load(m_aEntries[m_iSel][0]);
				else
					Accept();

				return;
			}

			case KeyCode.KC_BACK: Up(); return;
			case KeyCode.KC_UP: Select(m_iSel - 1); return;
			case KeyCode.KC_DOWN: Select(m_iSel + 1); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Up()
	{
		if (m_sDir != "/")
			Load(ROOT_AER_Filesystem.Parent(m_sDir));
	}

	//------------------------------------------------------------------------------------------------
	protected void Select(int i)
	{
		if (m_aEntries.IsEmpty())
			return;

		m_iSel = Math.ClampInt(i, 0, m_aEntries.Count() - 1);
		int lines = Math.Floor(m_fListH / ROW_H);
		if (m_iSel < m_iScroll)
			m_iScroll = m_iSel;
		else if (m_iSel >= m_iScroll + lines)
			m_iScroll = m_iSel - lines + 1;

		if (!IsDir(m_iSel) && !m_bFolders)
			ROOT_AER_UI.SetEdit(m_wName, ROOT_AER_Filesystem.BaseName(m_aEntries[m_iSel][0]));

		RenderList();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= ID_ROW && id < ID_ROW + m_aEntries.Count())
		{
			Select(id - ID_ROW);
			return true;
		}

		if (id >= ID_PLACE && id < ID_PLACE + 4)
		{
			array<string> places = {ROOT_AER_Filesystem.HomeOf(ROOT_AER_Client.Get().m_sUser), "/", "/mnt", "/tmp"};
			Load(places[id - ID_PLACE]);
			return true;
		}

		switch (id)
		{
			case ID_BG: return true;
			case ID_LIST:
			{
				m_iSel = -1;
				RenderList();
				return true;
			}

			case ID_UP: Up(); return true;
			case ID_OK: Accept(); return true;
			case ID_CANCEL: Cancel(); return true;
			case ID_NEWDIR:
			{
				if (m_Desktop)
					m_Desktop.ShowDialog(this, "picker_newdir", "New folder", "Name of the new folder:", true, "New folder", "Create");

				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (tag != "picker_newdir" || !ok || value.Trim().IsEmpty())
			return;

		array<string> args = {Join(m_sDir, value.Trim())};
		ROOT_AER_Client.Get().Request("fs_mkdir", args, this);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiDoubleClick(Widget w, int id)
	{
		if (id < ID_ROW || id >= ID_ROW + m_aEntries.Count())
			return false;

		int i = id - ID_ROW;
		if (IsDir(i))
		{
			Load(m_aEntries[i][0]);
			return true;
		}

		if (!m_bFolders)
		{
			ROOT_AER_UI.SetEdit(m_wName, ROOT_AER_Filesystem.BaseName(m_aEntries[i][0]));
			Accept();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiWheel(Widget w, int id, int wheel)
	{
		m_iScroll -= wheel * 3;
		RenderList();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (finished && id == ID_NAME)
		{
			Accept();
			return true;
		}

		return false;
	}
}
