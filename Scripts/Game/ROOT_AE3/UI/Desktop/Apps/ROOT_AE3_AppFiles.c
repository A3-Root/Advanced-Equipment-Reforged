//! File manager: back / forward / up, clickable address bar (click the empty part to type a path), search,
//! navigation pane, large icons or details view with sortable columns, context menus, new folder / text document,
//! cut / copy / paste, rename, delete to the recycle bin, shortcuts (symlinks), properties with owner and
//! permissions, password protected files.
//! Keys: Enter open, Backspace back, Delete, F2 rename, F5 refresh, arrows select, Ctrl+C / X / V.
class ROOT_AE3_AppFiles : ROOT_AE3_App
{
	protected static const int ID_BACK = 1;
	protected static const int ID_FWD = 2;
	protected static const int ID_UP = 3;
	protected static const int ID_REFRESH = 4;
	protected static const int ID_ADDR = 5;
	protected static const int ID_ADDR_EDIT = 6;
	protected static const int ID_SEARCH = 7;
	protected static const int ID_NEW = 8;
	protected static const int ID_CUT = 9;
	protected static const int ID_COPY = 10;
	protected static const int ID_PASTE = 11;
	protected static const int ID_RENAME = 12;
	protected static const int ID_DELETE = 13;
	protected static const int ID_PROPS = 14;
	protected static const int ID_VIEW = 15;
	protected static const int ID_PROPS_OK = 20;
	protected static const int ID_PROPS_CANCEL = 21;
	protected static const int ID_PROPS_BG = 22;
	protected static const int ID_OWNER = 23;
	protected static const int ID_CRUMB = 100;
	protected static const int ID_NAV = 200;
	protected static const int ID_PERM = 300;
	protected static const int M_OPEN = 1;
	protected static const int M_CUT = 2;
	protected static const int M_COPY = 3;
	protected static const int M_PASTE = 4;
	protected static const int M_RENAME = 5;
	protected static const int M_DELETE = 6;
	protected static const int M_PROPS = 7;
	protected static const int M_LINK = 8;
	protected static const int M_NEWDIR = 9;
	protected static const int M_NEWFILE = 10;
	protected static const int M_REFRESH = 11;
	protected static const int M_GRID = 12;
	protected static const int M_DETAILS = 13;
	protected static const int M_LOCATION = 14;
	protected static const int LIST = 1;
	protected static const float NAV_W = 176;
	protected static const float TOP_H = 88;

	protected static bool s_bGrid = true;
	protected static string s_sClip;
	protected static bool s_bCut;

	protected string m_sPath = "~";
	protected string m_sSearch;
	protected ref array<string> m_aBack = {};
	protected ref array<string> m_aFwd = {};
	protected ROOT_AE3_ListView m_List;
	protected FrameWidget m_wAddr;
	protected FrameWidget m_wProps;
	protected EditBoxWidget m_wAddrEdit;
	protected EditBoxWidget m_wSearch;
	protected EditBoxWidget m_wOwner;
	protected ref ROOT_AE3_ForwardListener m_AddrListener;
	protected ref ROOT_AE3_ForwardListener m_PropsListener;
	protected ref array<ref array<string>> m_aEntries = {};
	protected ref array<string> m_aCrumbs = {};
	protected ref array<string> m_aNavPaths = {};
	protected int m_iSortCol = -1;
	protected bool m_bSortDesc;
	protected string m_sPropsPath;
	protected string m_sPropsOwner;
	protected string m_sPropsPerms;
	protected ref array<bool> m_aPerm = {};
	protected ref array<string> m_aPropsEntry = {};
	protected float m_fAddrW;
	protected bool m_bNoLoad;

	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Files";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "folder";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(880, 560, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool NeedsFileBrowsing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string Home()
	{
		return ROOT_AE3_Filesystem.HomeOf(ROOT_AE3_Client.Get().m_sUser);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		if (!m_sArg.IsEmpty())
		{
			m_sPath = m_sArg;
			m_sArg = "";
		}

		m_wProps = null;
		m_wAddrEdit = null;
		float w = Width();
		float h = Height();
		FrameWidget c = GetContent();
		ROOT_AE3_UI.Rect(c, 0, 0, w, TOP_H, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.IconButton(c, 6, 6, 34, 32, "g_back", ID_BACK, this, 16);
		ROOT_AE3_UI.IconButton(c, 42, 6, 34, 32, "g_forward", ID_FWD, this, 16);
		ROOT_AE3_UI.IconButton(c, 78, 6, 34, 32, "g_up", ID_UP, this, 16);
		ROOT_AE3_UI.IconButton(c, 114, 6, 34, 32, "g_refresh", ID_REFRESH, this, 16);
		float searchW = Math.Min(220, w * 0.25);
		m_fAddrW = w - 156 - searchW - 14;
		m_wAddr = ROOT_AE3_UI.Frame(c, 156, 6, m_fAddrW, 32);
		ROOT_AE3_UI.Box(c, w - searchW - 8, 6, searchW, 32, ROOT_AE3_Theme.Field());
		ROOT_AE3_UI.Icon(c, w - searchW + 2, 14, 16, "g_search", ROOT_AE3_Theme.Muted());
		m_wSearch = ROOT_AE3_UI.Edit(c, w - searchW + 24, 9, searchW - 36, 26, "", ID_SEARCH, this);
		if (m_wSearch)
			m_wSearch.SetColor(ROOT_AE3_Theme.Text());

		ROOT_AE3_UI.Placeholder(m_wSearch, c, w - searchW + 24, 13, searchW - 36, "Search", ROOT_AE3_Theme.Muted());
		BuildCommands(w);
		ROOT_AE3_UI.Rect(c, 0, TOP_H, w, 1, ROOT_AE3_Theme.Border());
		BuildNav(h);
		m_List = List(LIST, NAV_W + 1, TOP_H + 1, w - NAV_W - 1, h - TOP_H - 1);
		m_List.m_bGrid = s_bGrid;
		m_List.m_sEmpty = "This folder is empty.";
		array<string> headers = {"Name", "Type", "Size", "Owner", "Permissions"};
		float lw = w - NAV_W - 1;
		array<float> widths = {lw * 0.42, lw * 0.18, lw * 0.12, lw * 0.13, lw * 0.15};
		m_List.SetColumns(headers, widths);
		m_List.m_iSortCol = m_iSortCol;
		m_List.m_bSortDesc = m_bSortDesc;
		RenderAddress();
		if (!m_bNoLoad)
			Load();
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildCommands(float w)
	{
		FrameWidget c = GetContent();
		array<string> labels = {"New", "|", "Cut", "Copy", "Paste", "Rename", "Delete", "|", "Properties"};
		array<string> icons = {"g_plus", "", "g_cut", "g_copy", "g_paste", "g_rename", "g_delete", "", "g_props"};
		array<int> ids = {ID_NEW, 0, ID_CUT, ID_COPY, ID_PASTE, ID_RENAME, ID_DELETE, 0, ID_PROPS};
		float x = 6;
		foreach (int i, string l : labels)
		{
			if (l == "|")
			{
				ROOT_AE3_UI.Rect(c, x + 3, 54, 1, 24, ROOT_AE3_Theme.Border());
				x += 8;
				continue;
			}

			float bw = 40 + l.Length() * 7;
			if (x + bw > w - 50)
				break;

			ROOT_AE3_UI.ItemButton(c, x, 48, bw, 36, icons[i], l, ids[i], this, 14);
			x += bw + 2;
		}

		string view = "g_list";
		if (!s_bGrid)
			view = "g_grid";

		ROOT_AE3_UI.IconButton(c, w - 46, 48, 40, 36, view, ID_VIEW, this, 16);
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildNav(float h)
	{
		FrameWidget c = GetContent();
		ROOT_AE3_UI.Rect(c, 0, TOP_H + 1, NAV_W, h - TOP_H - 1, ROOT_AE3_Theme.Sidebar());
		ROOT_AE3_UI.Rect(c, NAV_W, TOP_H + 1, 1, h - TOP_H - 1, ROOT_AE3_Theme.Border());
		array<string> labels = {"Home", "This computer", "Drives", "Temp", "Recycle Bin"};
		array<string> icons = {"folder", "computer", "drive", "folder", "trash"};
		m_aNavPaths.Clear();
		m_aNavPaths.Insert(Home());
		m_aNavPaths.Insert("/");
		m_aNavPaths.Insert("/mnt");
		m_aNavPaths.Insert("/tmp");
		m_aNavPaths.Insert("");
		float y = TOP_H + 10;
		foreach (int i, string l : labels)
		{
			Color bg = ROOT_AE3_UI.Rgb(0, 0, 0, 0);
			if (m_aNavPaths[i] == m_sPath)
				bg = ROOT_AE3_Theme.Select();

			ROOT_AE3_UI.ItemButton(c, 6, y, NAV_W - 12, 32, icons[i], l, ID_NAV + i, this, 14, null, bg);
			y += 34;
			if (i == 0)
				y += 8;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Address bar as clickable path parts.
	protected void RenderAddress()
	{
		if (!m_wAddr)
			return;

		ROOT_AE3_UI.Clear(m_wAddr);
		m_AddrListener = new ROOT_AE3_ForwardListener(this);
		m_wAddrEdit = null;
		float aw = m_fAddrW;
		ROOT_AE3_UI.Box(m_wAddr, 0, 0, aw, 32, ROOT_AE3_Theme.Field());
		ROOT_AE3_UI.Rect(m_wAddr, 1, 1, aw - 2, 30, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ID_ADDR, m_AddrListener);
		ROOT_AE3_UI.Icon(m_wAddr, 8, 7, 18, PathIcon(m_sPath));
		m_aCrumbs.Clear();
		array<string> names = {};
		if (!m_sSearch.IsEmpty())
		{
			names.Insert("Search results for \"" + m_sSearch + "\" in " + m_sPath);
			m_aCrumbs.Insert(m_sPath);
		}
		else
		{
			names.Insert("This computer");
			m_aCrumbs.Insert("/");
			array<string> parts = {};
			m_sPath.Split("/", parts, true);
			string acc = "";
			foreach (string p : parts)
			{
				acc += "/" + p;
				names.Insert(p);
				m_aCrumbs.Insert(acc);
			}
		}

		// Leading parts are dropped while the address does not fit.
		int first = 0;
		float total = 0;
		for (int i = names.Count() - 1; i >= 0; i--)
		{
			total += 30 + names[i].Length() * 7;
			if (total > aw - 50)
				break;

			first = i;
		}

		float x = 32;
		for (int i = first; i < names.Count(); i++)
		{
			float bw = 16 + names[i].Length() * 7;
			ROOT_AE3_UI.Button(m_wAddr, x, 3, bw, 26, names[i], ID_CRUMB + i, m_AddrListener, ROOT_AE3_UI.Rgb(0, 0, 0, 0), ROOT_AE3_Theme.Text(), 14);
			x += bw;
			if (i < names.Count() - 1)
			{
				ROOT_AE3_UI.Icon(m_wAddr, x + 2, 11, 10, "g_chevron", ROOT_AE3_Theme.Muted());
				x += 14;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Address bar as an edit box with the full path.
	protected void EditAddress()
	{
		ROOT_AE3_UI.Clear(m_wAddr);
		m_AddrListener = new ROOT_AE3_ForwardListener(this);
		float aw = m_fAddrW;
		ROOT_AE3_UI.Box(m_wAddr, 0, 0, aw, 32, ROOT_AE3_Theme.Field(), ROOT_AE3_Theme.Accent());
		m_wAddrEdit = ROOT_AE3_UI.Edit(m_wAddr, 10, 3, aw - 20, 26, m_sPath, ID_ADDR_EDIT, m_AddrListener);
		if (!m_wAddrEdit)
			return;

		m_wAddrEdit.SetColor(ROOT_AE3_Theme.Text());
		GetGame().GetWorkspace().SetFocusedWidget(m_wAddrEdit);
		m_wAddrEdit.ActivateWriteMode();
	}

	//------------------------------------------------------------------------------------------------
	protected string PathIcon(string path)
	{
		if (path == "/")
			return "computer";

		if (path.StartsWith("/mnt/") || path == "/mnt")
			return "drive";

		return "folder";
	}

	//------------------------------------------------------------------------------------------------
	protected void Load()
	{
		Request1("fs_list", m_sPath);
	}

	//------------------------------------------------------------------------------------------------
	protected void Navigate(string path)
	{
		if (path.IsEmpty())
			return;

		if (path != m_sPath || !m_sSearch.IsEmpty())
		{
			m_aBack.Insert(m_sPath);
			m_aFwd.Clear();
		}

		m_sSearch = "";
		m_sPath = path;
		Load();
	}

	//------------------------------------------------------------------------------------------------
	protected void GoBack()
	{
		if (m_aBack.IsEmpty())
			return;

		m_aFwd.Insert(m_sPath);
		m_sPath = m_aBack[m_aBack.Count() - 1];
		m_aBack.Remove(m_aBack.Count() - 1);
		m_sSearch = "";
		Load();
	}

	//------------------------------------------------------------------------------------------------
	protected void GoForward()
	{
		if (m_aFwd.IsEmpty())
			return;

		m_aBack.Insert(m_sPath);
		m_sPath = m_aFwd[m_aFwd.Count() - 1];
		m_aFwd.Remove(m_aFwd.Count() - 1);
		m_sSearch = "";
		Load();
	}

	//------------------------------------------------------------------------------------------------
	protected array<string> SelectedEntry()
	{
		if (!m_List)
			return null;

		int i = m_List.m_iSelected;
		if (i < 0 || i >= m_aEntries.Count())
			return null;

		return m_aEntries[i];
	}

	//------------------------------------------------------------------------------------------------
	protected string SelectedPath()
	{
		array<string> e = SelectedEntry();
		if (!e)
			return "";

		return e[0];
	}

	//------------------------------------------------------------------------------------------------
	protected static string Join(string dir, string name)
	{
		if (dir.EndsWith("/"))
			return dir + name;

		return dir + "/" + name;
	}

	//------------------------------------------------------------------------------------------------
	protected static string TypeName(array<string> e)
	{
		if (e[1] == "dir")
			return "File folder";

		if (e[1] == "link")
			return "Shortcut";

		if (e[1] == "search")
			return "Search result";

		if (e[5] == "1")
			return "Image";

		if (e[6] == "1")
			return "Protected file";

		if (e[7] == "1")
			return "Program";

		if (ROOT_AE3_Filesystem.BaseName(e[0]).Contains("."))
			return "Text document";

		return "File";
	}

	//------------------------------------------------------------------------------------------------
	//! Permission letters without the type character.
	protected static string PermsOf(array<string> e)
	{
		if (e[4].Length() < 2)
			return e[4];

		return e[4].Substring(1, e[4].Length() - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected static string IconOf(array<string> e)
	{
		if (e[1] == "search")
			return "file";

		return ROOT_AE3_Icons.ForFile(e[1], e[5] == "1", e[6] == "1", e[7] == "1", ROOT_AE3_Filesystem.BaseName(e[0]));
	}

	//------------------------------------------------------------------------------------------------
	protected static float SizeValue(string s)
	{
		array<string> p = {};
		s.Split(" ", p, true);
		if (p.Count() < 2)
			return -1;

		float v = p[0].ToFloat();
		switch (p[1])
		{
			case "KB": return v * 1024;
			case "MB": return v * 1048576;
			case "GB": return v * 1073741824;
		}

		return v;
	}

	//------------------------------------------------------------------------------------------------
	//! Negative when a sorts before b. Folders come first unless sorting by type.
	protected int Compare(array<string> a, array<string> b)
	{
		bool da = a[1] == "dir";
		bool db = b[1] == "dir";
		if (da != db && m_iSortCol != 1)
		{
			if (da)
				return -1;

			return 1;
		}

		int r;
		switch (m_iSortCol)
		{
			case 1: r = TypeName(a).Compare(TypeName(b)); break;
			case 2:
			{
				float sa = SizeValue(a[2]);
				float sb = SizeValue(b[2]);
				if (sa < sb)
					r = -1;
				else if (sa > sb)
					r = 1;

				break;
			}

			case 3: r = a[3].Compare(b[3]); break;
			case 4: r = a[4].Compare(b[4]); break;
		}

		if (r == 0)
		{
			string na = ROOT_AE3_Filesystem.BaseName(a[0]);
			string nb = ROOT_AE3_Filesystem.BaseName(b[0]);
			na.ToLower();
			nb.ToLower();
			r = na.Compare(nb);
		}

		if (m_bSortDesc)
			return -r;

		return r;
	}

	//------------------------------------------------------------------------------------------------
	protected void SortEntries()
	{
		for (int i = 1; i < m_aEntries.Count(); i++)
		{
			int j = i;
			while (j > 0 && Compare(m_aEntries[j - 1], m_aEntries[j]) > 0)
			{
				m_aEntries.SwapItems(j - 1, j);
				j--;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowEntries(string selectName = "")
	{
		SortEntries();
		m_List.Clear();
		m_List.m_bGrid = s_bGrid;
		m_List.m_iSortCol = m_iSortCol;
		m_List.m_bSortDesc = m_bSortDesc;
		foreach (int i, array<string> e : m_aEntries)
		{
			string name = ROOT_AE3_Filesystem.BaseName(e[0]);
			if (e[1] == "search")
				name = e[0];

			array<string> row = {name, TypeName(e), e[2], e[3], PermsOf(e)};
			m_List.AddRow(row, IconOf(e));
			if (!selectName.IsEmpty() && name == selectName)
				m_List.m_iSelected = i;
		}

		m_List.Render();
		UpdateStatus();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateStatus()
	{
		string s = string.Format("%1 items", m_aEntries.Count());
		array<string> e = SelectedEntry();
		if (e)
		{
			s += "     1 item selected";
			if (e[1] != "dir" && !e[2].IsEmpty() && e[2] != "-")
				s += "  " + e[2];
		}

		if (!s_sClip.IsEmpty())
		{
			string op = "Copied: ";
			if (s_bCut)
				op = "Cut: ";

			s += "     " + op + ROOT_AE3_Filesystem.BaseName(s_sClip);
		}

		Status(s);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens a folder here, other files in the fitting app.
	protected void OpenEntry(array<string> e)
	{
		if (e[1] == "search")
		{
			Navigate(ROOT_AE3_Filesystem.Parent(e[0]));
			return;
		}

		if (e[1] == "dir")
		{
			Navigate(e[0]);
			return;
		}

		if (e[5] == "1")
		{
			m_Desktop.OpenApp(ROOT_AE3_AppMedia, e[0]);
			return;
		}

		if (e[6] == "1")
		{
			m_Desktop.ShowDialog(this, "unlock", "Protected file", "\"" + ROOT_AE3_Filesystem.BaseName(e[0]) + "\" is password protected. Enter the password to remove the protection.", true, "", "Unlock", true);
			return;
		}

		if (e[7] == "1")
		{
			Request1("run_file", e[0]);
			return;
		}

		m_Desktop.OpenApp(ROOT_AE3_AppNotepad, e[0]);
	}

	// Actions ---------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void DoClip(bool cut)
	{
		string sel = SelectedPath();
		if (sel.IsEmpty())
		{
			Status("Select a file or folder first.");
			return;
		}

		s_sClip = sel;
		s_bCut = cut;
		UpdateStatus();
	}

	//------------------------------------------------------------------------------------------------
	protected void DoPaste()
	{
		if (s_sClip.IsEmpty())
		{
			Status("Nothing to paste. Copy or cut a file first.");
			return;
		}

		string name = ROOT_AE3_Filesystem.BaseName(s_sClip);
		if (ROOT_AE3_Filesystem.Parent(s_sClip) == m_sPath)
		{
			if (s_bCut)
				return;

			name += " - Copy";
		}

		string dst = Join(m_sPath, name);
		if (s_bCut)
		{
			Request2("fs_move", s_sClip, dst);
			s_sClip = "";
			Status("Moving...");
			return;
		}

		Request2("fs_copy", s_sClip, dst);
		Status("Copying " + name + "...");
	}

	//------------------------------------------------------------------------------------------------
	protected void DoRename()
	{
		string sel = SelectedPath();
		if (!sel.IsEmpty())
			m_Desktop.ShowDialog(this, "rename", "Rename", "New name for \"" + ROOT_AE3_Filesystem.BaseName(sel) + "\":", true, ROOT_AE3_Filesystem.BaseName(sel), "Rename");
	}

	//------------------------------------------------------------------------------------------------
	protected void DoDelete()
	{
		string sel = SelectedPath();
		if (!sel.IsEmpty())
			Request1("fs_delete", sel);
	}

	//------------------------------------------------------------------------------------------------
	protected void NewMenu()
	{
		array<string> labels = {"Folder", "Text document"};
		array<int> ids = {M_NEWDIR, M_NEWFILE};
		array<string> icons = {"folder", "file_text"};
		m_Desktop.ShowMenu(this, labels, ids, icons);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuItem(int id)
	{
		array<string> e = SelectedEntry();
		switch (id)
		{
			case M_OPEN:
			{
				if (e)
					OpenEntry(e);

				return;
			}

			case M_LOCATION:
			{
				if (e)
					Navigate(ROOT_AE3_Filesystem.Parent(e[0]));

				return;
			}

			case M_CUT: DoClip(true); return;
			case M_COPY: DoClip(false); return;
			case M_PASTE: DoPaste(); return;
			case M_RENAME: DoRename(); return;
			case M_DELETE: DoDelete(); return;
			case M_PROPS:
			{
				if (e)
					ShowProps(e);

				return;
			}

			case M_LINK:
			{
				if (e)
					Request2("fs_symlink", e[0], Join(m_sPath, ROOT_AE3_Filesystem.BaseName(e[0]) + " - Shortcut"));

				return;
			}

			case M_NEWDIR: m_Desktop.ShowDialog(this, "newdir", "New folder", "Name of the new folder:", true, "New folder", "Create"); return;
			case M_NEWFILE: m_Desktop.ShowDialog(this, "newfile", "New text document", "Name of the new document:", true, "New text document.txt", "Create"); return;
			case M_REFRESH: Load(); return;
			case M_GRID:
			case M_DETAILS:
			{
				s_bGrid = id == M_GRID;
				Redraw();
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDialog(string tag, bool ok, string value)
	{
		if (!ok)
			return;

		string v = value.Trim();
		string sel = SelectedPath();
		switch (tag)
		{
			case "newdir":
			{
				if (!v.IsEmpty())
					Request1("fs_mkdir", Join(m_sPath, v));

				return;
			}

			case "newfile":
			{
				if (!v.IsEmpty())
					Request2("fs_save", Join(m_sPath, v), "");

				return;
			}

			case "rename":
			{
				if (!v.IsEmpty() && !sel.IsEmpty())
					Request2("fs_move", sel, Join(ROOT_AE3_Filesystem.Parent(sel), v));

				return;
			}

			case "unlock":
			{
				array<string> args = {sel, value, "1"};
				Request("fs_unlock", args);
				return;
			}
		}
	}

	// Properties ------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void ShowProps(array<string> e)
	{
		if (e[1] == "search")
			return;

		m_aPropsEntry.Copy(e);
		m_sPropsPath = e[0];
		m_sPropsOwner = e[3];
		m_sPropsPerms = PermsOf(e);
		m_wOwner = null;
		m_aPerm.Clear();
		for (int i = 0; i < 6; i++)
		{
			m_aPerm.Insert(i < m_sPropsPerms.Length() && m_sPropsPerms.Get(i) != "-");
		}

		RenderProps("");
	}

	//------------------------------------------------------------------------------------------------
	protected void RenderProps(string owner)
	{
		string ownerText = owner;
		if (m_wOwner)
			ownerText = m_wOwner.GetText();

		if (m_wProps)
			m_wProps.RemoveFromHierarchy();

		if (ownerText.IsEmpty())
			ownerText = m_sPropsOwner;

		array<string> e = m_aPropsEntry;
		float w = Width();
		float h = Height();
		float pw = 400;
		float ph = 372;
		float x = (w - pw) * 0.5;
		float y = Math.Max(4, (h - ph) * 0.5);
		m_PropsListener = new ROOT_AE3_ForwardListener(this);
		m_wProps = ROOT_AE3_UI.Frame(GetContent(), 0, 0, w, h, false);
		m_wProps.SetZOrder(1000);
		ROOT_AE3_UI.Rect(m_wProps, 0, 0, w, h, ROOT_AE3_UI.Rgb(0, 0, 0, 0.25), ID_PROPS_BG, m_PropsListener);
		ROOT_AE3_UI.Shadow(m_wProps, x, y, pw, ph, 12);
		ROOT_AE3_UI.Box(m_wProps, x, y, pw, ph, ROOT_AE3_Theme.Panel());
		ROOT_AE3_UI.Icon(m_wProps, x + 20, y + 18, 40, IconOf(e));
		ROOT_AE3_UI.Text(m_wProps, x + 74, y + 20, pw - 94, 20, ROOT_AE3_Filesystem.BaseName(e[0]), 16, ROOT_AE3_Theme.Text());
		ROOT_AE3_UI.Text(m_wProps, x + 74, y + 42, pw - 94, 18, TypeName(e), 13, ROOT_AE3_Theme.Muted());
		ROOT_AE3_UI.Rect(m_wProps, x + 20, y + 72, pw - 40, 1, ROOT_AE3_Theme.Border());
		array<string> keys = {"Location", "Size"};
		array<string> vals = {ROOT_AE3_Filesystem.Parent(e[0]), e[2]};
		float ry = y + 84;
		foreach (int i, string k : keys)
		{
			ROOT_AE3_UI.Text(m_wProps, x + 20, ry, 100, 18, k, 14, ROOT_AE3_Theme.Muted());
			ROOT_AE3_UI.Text(m_wProps, x + 120, ry, pw - 140, 18, vals[i], 14, ROOT_AE3_Theme.Text());
			ry += 26;
		}

		ROOT_AE3_UI.Text(m_wProps, x + 20, ry + 6, 100, 18, "Owner", 14, ROOT_AE3_Theme.Muted());
		ROOT_AE3_UI.Box(m_wProps, x + 120, ry, pw - 140, 30, ROOT_AE3_Theme.Field());
		m_wOwner = ROOT_AE3_UI.Edit(m_wProps, x + 128, ry + 2, pw - 156, 26, ownerText, ID_OWNER, m_PropsListener);
		if (m_wOwner)
			m_wOwner.SetColor(ROOT_AE3_Theme.Text());

		ry += 44;
		ROOT_AE3_UI.Text(m_wProps, x + 20, ry, 200, 18, "Permissions", 14, ROOT_AE3_Theme.Text());
		ry += 26;
		array<string> cols = {"Read", "Write", "Run / open"};
		foreach (int c, string col : cols)
		{
			ROOT_AE3_UI.Text(m_wProps, x + 140 + c * 82, ry, 80, 18, col, 13, ROOT_AE3_Theme.Muted(), false, false, 1);
		}

		ry += 24;
		array<string> rows = {"Owner", "Everyone"};
		foreach (int r, string row : rows)
		{
			ROOT_AE3_UI.Text(m_wProps, x + 20, ry + 4, 110, 18, row, 14, ROOT_AE3_Theme.Text());
			for (int c = 0; c < 3; c++)
			{
				int bit = r * 3 + c;
				float bx = x + 140 + c * 82 + 30;
				Color fill = ROOT_AE3_Theme.Field();
				if (m_aPerm[bit])
					fill = ROOT_AE3_Theme.Accent();

				ROOT_AE3_UI.Rect(m_wProps, bx, ry, 22, 22, ROOT_AE3_Theme.Muted());
				ROOT_AE3_UI.Hit(m_wProps, bx + 1, ry + 1, 20, 20, ID_PERM + bit, m_PropsListener, fill, ROOT_AE3_Theme.Hover());
				if (m_aPerm[bit])
					ROOT_AE3_UI.Icon(m_wProps, bx + 4, ry + 4, 14, "g_check", ROOT_AE3_UI.Rgb(1, 1, 1));
			}

			ry += 32;
		}

		ROOT_AE3_UI.Rect(m_wProps, x + 1, y + ph - 60, pw - 2, 59, ROOT_AE3_Theme.Bar());
		ROOT_AE3_UI.Button(m_wProps, x + pw - 236, y + ph - 46, 104, 32, "OK", ID_PROPS_OK, m_PropsListener, ROOT_AE3_Theme.Accent(), ROOT_AE3_UI.Rgb(1, 1, 1), 14);
		ROOT_AE3_UI.Rect(m_wProps, x + pw - 124, y + ph - 46, 104, 32, ROOT_AE3_Theme.Border());
		ROOT_AE3_UI.Button(m_wProps, x + pw - 123, y + ph - 45, 102, 30, "Cancel", ID_PROPS_CANCEL, m_PropsListener, ROOT_AE3_Theme.Button(), ROOT_AE3_Theme.Text(), 14);
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseProps(bool apply)
	{
		if (apply)
		{
			string perms = "";
			array<string> letters = {"r", "w", "x", "r", "w", "x"};
			for (int i = 0; i < 6; i++)
			{
				if (m_aPerm[i])
					perms += letters[i];
				else
					perms += "-";
			}

			if (perms != m_sPropsPerms)
				Request2("fs_chmod", m_sPropsPath, perms);

			if (m_wOwner)
			{
				string owner = m_wOwner.GetText().Trim();
				if (!owner.IsEmpty() && owner != m_sPropsOwner)
					Request2("fs_chown", m_sPropsPath, owner);
			}
		}

		if (m_wProps)
			m_wProps.RemoveFromHierarchy();

		m_wProps = null;
		m_wOwner = null;
	}

	// Events ----------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void OnGuiReply(string cmd, bool ok, array<string> payload)
	{
		if (!ok)
		{
			Status(Arg(payload, 0));
			if (cmd == "fs_list" && !m_aBack.IsEmpty())
			{
				// The folder could not be opened: stay in the previous one.
				m_sPath = m_aBack[m_aBack.Count() - 1];
				m_aBack.Remove(m_aBack.Count() - 1);
				RenderAddress();
			}

			return;
		}

		switch (cmd)
		{
			case "fs_list":
			{
				m_sPath = Arg(payload, 0);
				m_aEntries.Clear();
				for (int i = 1; i < payload.Count(); i++)
				{
					array<string> e = ROOT_AE3_Codec.SplitSub(payload[i]);
					if (e.Count() >= 8)
						m_aEntries.Insert(e);
				}

				string title = ROOT_AE3_Filesystem.BaseName(m_sPath);
				if (m_sPath == "/")
					title = "This computer";

				m_Window.SetTitle(title);
				Redraw();
				return;
			}

			case "fs_search":
			{
				m_aEntries.Clear();
				foreach (string p : payload)
				{
					array<string> e = {p, "search", "", "", "", "0", "0", "0"};
					m_aEntries.Insert(e);
				}

				RenderAddress();
				m_List.m_sEmpty = "No items match your search.";
				ShowEntries();
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
				if (m_sSearch.IsEmpty())
					Load();

				return;
			}
		}

		Status(Arg(payload, 0));
	}

	//------------------------------------------------------------------------------------------------
	//! Rebuild keeps the listing; only fs_list replies request a new one.
	override void Redraw()
	{
		m_aLists.Clear();
		ReleaseHandlers();
		ROOT_AE3_UI.Clear(GetContent());
		m_bNoLoad = true;
		Build();
		m_bNoLoad = false;
		ShowEntries();
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
		UpdateStatus();
	}

	//------------------------------------------------------------------------------------------------
	override void OnListSort(int listId, int column)
	{
		if (m_iSortCol == column)
			m_bSortDesc = !m_bSortDesc;
		else
		{
			m_iSortCol = column;
			m_bSortDesc = false;
		}

		string sel = "";
		array<string> e = SelectedEntry();
		if (e)
			sel = ROOT_AE3_Filesystem.BaseName(e[0]);

		ShowEntries(sel);
	}

	//------------------------------------------------------------------------------------------------
	override void OnListContext(int listId, int row)
	{
		if (row < 0)
		{
			array<string> bgLabels = {"Large icons", "Details", "-", "Refresh", "-", "Paste", "-", "New folder", "New text document"};
			array<int> bgIds = {M_GRID, M_DETAILS, 0, M_REFRESH, 0, M_PASTE, 0, M_NEWDIR, M_NEWFILE};
			array<string> bgIcons = {"g_grid", "g_list", "", "g_refresh", "", "g_paste", "", "folder", "file_text"};
			m_Desktop.ShowMenu(this, bgLabels, bgIds, bgIcons);
			return;
		}

		if (row >= m_aEntries.Count())
			return;

		if (m_aEntries[row][1] == "search")
		{
			array<string> sLabels = {"Open file location"};
			array<int> sIds = {M_LOCATION};
			array<string> sIcons = {"folder"};
			m_Desktop.ShowMenu(this, sLabels, sIds, sIcons);
			return;
		}

		array<string> labels = {"Open", "-", "Cut", "Copy", "-", "Create shortcut", "Rename", "Delete", "-", "Properties"};
		array<int> ids = {M_OPEN, 0, M_CUT, M_COPY, 0, M_LINK, M_RENAME, M_DELETE, 0, M_PROPS};
		array<string> icons = {"g_open", "", "g_cut", "g_copy", "", "g_link", "g_rename", "g_delete", "", "g_props"};
		m_Desktop.ShowMenu(this, labels, ids, icons);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnKey(KeyCode key, bool ctrl)
	{
		if (m_wProps)
			return false;

		if (ctrl)
		{
			switch (key)
			{
				case KeyCode.KC_C: DoClip(false); return true;
				case KeyCode.KC_X: DoClip(true); return true;
				case KeyCode.KC_V: DoPaste(); return true;
			}

			return false;
		}

		switch (key)
		{
			case KeyCode.KC_RETURN:
			case KeyCode.KC_NUMPADENTER:
			{
				array<string> e = SelectedEntry();
				if (e)
					OpenEntry(e);

				return true;
			}

			case KeyCode.KC_BACK: GoBack(); return true;
			case KeyCode.KC_DELETE: DoDelete(); return true;
			case KeyCode.KC_F2: DoRename(); return true;
			case KeyCode.KC_F5: Load(); return true;
			case KeyCode.KC_UP: m_List.Move(-1); return true;
			case KeyCode.KC_DOWN: m_List.Move(1); return true;
			case KeyCode.KC_LEFT: m_List.Move(-1, false); return true;
			case KeyCode.KC_RIGHT: m_List.Move(1, false); return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiChange(Widget w, int id, bool finished)
	{
		if (!finished)
			return false;

		if (id == ID_ADDR_EDIT && m_wAddrEdit)
		{
			string p = m_wAddrEdit.GetText().Trim();
			GetGame().GetWorkspace().SetFocusedWidget(null);
			if (p.IsEmpty())
				RenderAddress();
			else
				Navigate(p);

			return true;
		}

		if (id == ID_SEARCH && m_wSearch)
		{
			string q = m_wSearch.GetText().Trim();
			GetGame().GetWorkspace().SetFocusedWidget(null);
			if (q.IsEmpty())
			{
				m_sSearch = "";
				Load();
				return true;
			}

			m_sSearch = q;
			Request2("fs_search", m_sPath, q);
			Status("Searching...");
			return true;
		}

		if (id == ID_OWNER)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id >= ID_PERM && id < ID_PERM + 6)
		{
			m_aPerm[id - ID_PERM] = !m_aPerm[id - ID_PERM];
			RenderProps("");
			return true;
		}

		if (id >= ID_CRUMB && id < ID_CRUMB + m_aCrumbs.Count())
		{
			Navigate(m_aCrumbs[id - ID_CRUMB]);
			return true;
		}

		if (id >= ID_NAV && id < ID_NAV + m_aNavPaths.Count())
		{
			string target = m_aNavPaths[id - ID_NAV];
			if (target.IsEmpty())
				m_Desktop.OpenApp(ROOT_AE3_AppRecycleBin);
			else
				Navigate(target);

			return true;
		}

		switch (id)
		{
			case ID_BACK: GoBack(); return true;
			case ID_FWD: GoForward(); return true;
			case ID_UP: Navigate(ROOT_AE3_Filesystem.Parent(m_sPath)); return true;
			case ID_REFRESH: Load(); return true;
			case ID_ADDR: EditAddress(); return true;
			case ID_NEW: NewMenu(); return true;
			case ID_CUT: DoClip(true); return true;
			case ID_COPY: DoClip(false); return true;
			case ID_PASTE: DoPaste(); return true;
			case ID_RENAME: DoRename(); return true;
			case ID_DELETE: DoDelete(); return true;
			case ID_PROPS:
			{
				array<string> e = SelectedEntry();
				if (e)
					ShowProps(e);
				else
					Status("Select a file or folder first.");

				return true;
			}

			case ID_VIEW:
			{
				s_bGrid = !s_bGrid;
				Redraw();
				return true;
			}

			case ID_PROPS_OK: CloseProps(true); return true;
			case ID_PROPS_CANCEL: CloseProps(false); return true;
			case ID_PROPS_BG: return true;
		}

		return false;
	}
}
