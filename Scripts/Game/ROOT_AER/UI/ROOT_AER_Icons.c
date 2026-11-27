//! Desktop icon textures in UI/Textures/ROOT_AER/Icons (tools/gen_icons.py). Names starting with g_ are white glyphs.
class ROOT_AER_Icons
{
	protected static ref map<string, ResourceName> s_mIcons;

	//------------------------------------------------------------------------------------------------
	static bool Has(string name)
	{
		if (!s_mIcons)
			Init();

		return s_mIcons.Contains(name);
	}

	//------------------------------------------------------------------------------------------------
	static ResourceName Get(string name)
	{
		if (!s_mIcons)
			Init();

		return s_mIcons.Get(name);
	}

	//------------------------------------------------------------------------------------------------
	//! Icon of a file list entry: type dir / link / file, media, locked, program flags.
	static string ForFile(string type, bool media, bool locked, bool program, string name)
	{
		if (type == "dir")
			return "folder";

		if (type == "link")
			return "file_link";

		if (media)
			return "file_image";

		if (locked)
			return "file_locked";

		if (program)
			return "file_program";

		if (name.Contains("."))
			return "file_text";

		return "file";
	}

	//------------------------------------------------------------------------------------------------
	protected static void Init()
	{
		s_mIcons = new map<string, ResourceName>();
		s_mIcons.Insert("browser", "{A1695289B86B27AE}UI/Textures/ROOT_AER/Icons/browser.edds");
		s_mIcons.Insert("calculator", "{D455D35BAC6BC7E0}UI/Textures/ROOT_AER/Icons/calculator.edds");
		s_mIcons.Insert("calendar", "{66745900C3093F4D}UI/Textures/ROOT_AER/Icons/calendar.edds");
		s_mIcons.Insert("cctv", "{B5D922161CDD9B87}UI/Textures/ROOT_AER/Icons/cctv.edds");
		s_mIcons.Insert("computer", "{5EE2D31606BD84AA}UI/Textures/ROOT_AER/Icons/computer.edds");
		s_mIcons.Insert("drive", "{562E3913B19EA1A6}UI/Textures/ROOT_AER/Icons/drive.edds");
		s_mIcons.Insert("file", "{991B18D45B1382AF}UI/Textures/ROOT_AER/Icons/file.edds");
		s_mIcons.Insert("file_image", "{CA5E49972160DA8E}UI/Textures/ROOT_AER/Icons/file_image.edds");
		s_mIcons.Insert("file_link", "{EFC36DFF6C71C9C3}UI/Textures/ROOT_AER/Icons/file_link.edds");
		s_mIcons.Insert("file_locked", "{E139F1B4C5930EEB}UI/Textures/ROOT_AER/Icons/file_locked.edds");
		s_mIcons.Insert("file_program", "{8CAE5DE977B32DC6}UI/Textures/ROOT_AER/Icons/file_program.edds");
		s_mIcons.Insert("file_text", "{5823189A58699FCE}UI/Textures/ROOT_AER/Icons/file_text.edds");
		s_mIcons.Insert("folder", "{E9A19B35F0E7F119}UI/Textures/ROOT_AER/Icons/folder.edds");
		s_mIcons.Insert("g_back", "{F8548B11DE0AAE82}UI/Textures/ROOT_AER/Icons/g_back.edds");
		s_mIcons.Insert("g_battery", "{F7D914FEAA8FBA79}UI/Textures/ROOT_AER/Icons/g_battery.edds");
		s_mIcons.Insert("g_check", "{279DCEB6A3D8F87C}UI/Textures/ROOT_AER/Icons/g_check.edds");
		s_mIcons.Insert("g_chevron", "{82A74ED4E9C1E52E}UI/Textures/ROOT_AER/Icons/g_chevron.edds");
		s_mIcons.Insert("g_chevron_down", "{5E090E5CBE3493D0}UI/Textures/ROOT_AER/Icons/g_chevron_down.edds");
		s_mIcons.Insert("g_close", "{6D84E4C0974E0D34}UI/Textures/ROOT_AER/Icons/g_close.edds");
		s_mIcons.Insert("g_copy", "{349E90EB040BD3AB}UI/Textures/ROOT_AER/Icons/g_copy.edds");
		s_mIcons.Insert("g_cut", "{E0DD8030AE1C66BD}UI/Textures/ROOT_AER/Icons/g_cut.edds");
		s_mIcons.Insert("g_delete", "{93C0AF380CD7CD97}UI/Textures/ROOT_AER/Icons/g_delete.edds");
		s_mIcons.Insert("g_dot", "{318D98E97E12F525}UI/Textures/ROOT_AER/Icons/g_dot.edds");
		s_mIcons.Insert("g_forward", "{9EFB6EEBC054983C}UI/Textures/ROOT_AER/Icons/g_forward.edds");
		s_mIcons.Insert("g_grid", "{28C7C928900A4DBF}UI/Textures/ROOT_AER/Icons/g_grid.edds");
		s_mIcons.Insert("g_home", "{3A7B0DB16B2E664B}UI/Textures/ROOT_AER/Icons/g_home.edds");
		s_mIcons.Insert("g_link", "{93E71CE08357C663}UI/Textures/ROOT_AER/Icons/g_link.edds");
		s_mIcons.Insert("g_list", "{E25AF28E349F27E6}UI/Textures/ROOT_AER/Icons/g_list.edds");
		s_mIcons.Insert("g_max", "{3414B346D69B4460}UI/Textures/ROOT_AER/Icons/g_max.edds");
		s_mIcons.Insert("g_min", "{27B231CF6B8B2873}UI/Textures/ROOT_AER/Icons/g_min.edds");
		s_mIcons.Insert("g_newfile", "{168AAC823BCD2631}UI/Textures/ROOT_AER/Icons/g_newfile.edds");
		s_mIcons.Insert("g_newfolder", "{C6AFADDD2296C50C}UI/Textures/ROOT_AER/Icons/g_newfolder.edds");
		s_mIcons.Insert("g_open", "{75E6FCF150DA99B2}UI/Textures/ROOT_AER/Icons/g_open.edds");
		s_mIcons.Insert("g_paste", "{588323D33FA08E84}UI/Textures/ROOT_AER/Icons/g_paste.edds");
		s_mIcons.Insert("g_plug", "{079481ADD9E35140}UI/Textures/ROOT_AER/Icons/g_plug.edds");
		s_mIcons.Insert("g_plus", "{4C644C82218742B1}UI/Textures/ROOT_AER/Icons/g_plus.edds");
		s_mIcons.Insert("g_power", "{E59D7A1297A81360}UI/Textures/ROOT_AER/Icons/g_power.edds");
		s_mIcons.Insert("g_props", "{564F1021F4FF420E}UI/Textures/ROOT_AER/Icons/g_props.edds");
		s_mIcons.Insert("g_refresh", "{FE81E938F65372C6}UI/Textures/ROOT_AER/Icons/g_refresh.edds");
		s_mIcons.Insert("g_rename", "{B8200528E5035D90}UI/Textures/ROOT_AER/Icons/g_rename.edds");
		s_mIcons.Insert("g_restore", "{BC9BB1CD7EE2969D}UI/Textures/ROOT_AER/Icons/g_restore.edds");
		s_mIcons.Insert("g_search", "{E67992098F05E01B}UI/Textures/ROOT_AER/Icons/g_search.edds");
		s_mIcons.Insert("g_up", "{BEAF67681D0FE472}UI/Textures/ROOT_AER/Icons/g_up.edds");
		s_mIcons.Insert("g_wifi", "{B55D354F146CC88B}UI/Textures/ROOT_AER/Icons/g_wifi.edds");
		s_mIcons.Insert("games", "{F071282648B00844}UI/Textures/ROOT_AER/Icons/games.edds");
		s_mIcons.Insert("games_board", "{AA3BCC0E56D69473}UI/Textures/ROOT_AER/Icons/games_board.edds");
		s_mIcons.Insert("info", "{5D30E0172E523695}UI/Textures/ROOT_AER/Icons/info.edds");
		s_mIcons.Insert("logo", "{E693820BA87C8B7F}UI/Textures/ROOT_AER/Icons/logo.edds");
		s_mIcons.Insert("start", "{70849521CB5D1C82}UI/Textures/ROOT_AER/Icons/start.edds");
		s_mIcons.Insert("g_start_tac", "{B731FA669A153BDF}UI/Textures/ROOT_AER/Icons/g_start_tac.edds");
		s_mIcons.Insert("mail", "{C8545DA12039FF07}UI/Textures/ROOT_AER/Icons/mail.edds");
		s_mIcons.Insert("map", "{7280B2B746292665}UI/Textures/ROOT_AER/Icons/map.edds");
		s_mIcons.Insert("messenger", "{1604823747E741D9}UI/Textures/ROOT_AER/Icons/messenger.edds");
		s_mIcons.Insert("network", "{5FEB5EDF7D002794}UI/Textures/ROOT_AER/Icons/network.edds");
		s_mIcons.Insert("notepad", "{E96F608D487BA25B}UI/Textures/ROOT_AER/Icons/notepad.edds");
		s_mIcons.Insert("photos", "{32A01CA7E08188F2}UI/Textures/ROOT_AER/Icons/photos.edds");
		s_mIcons.Insert("ping", "{BDF2023164C4608B}UI/Textures/ROOT_AER/Icons/ping.edds");
		s_mIcons.Insert("remote", "{E522784038BB68C7}UI/Textures/ROOT_AER/Icons/remote.edds");
		s_mIcons.Insert("settings", "{0E13E090F559C9EC}UI/Textures/ROOT_AER/Icons/settings.edds");
		s_mIcons.Insert("snake", "{4D1638A1EC5E503C}UI/Textures/ROOT_AER/Icons/snake.edds");
		s_mIcons.Insert("terminal", "{B6D51208DD9FD029}UI/Textures/ROOT_AER/Icons/terminal.edds");
		s_mIcons.Insert("trash", "{CDA196E1DD405615}UI/Textures/ROOT_AER/Icons/trash.edds");
		s_mIcons.Insert("trash_full", "{BE1C281F1561D542}UI/Textures/ROOT_AER/Icons/trash_full.edds");
		s_mIcons.Insert("user", "{A9D8D928A62AFAE2}UI/Textures/ROOT_AER/Icons/user.edds");
	}
}
