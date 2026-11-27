//! Mod settings. Defaults below; a mission overrides them through the mission header (m_ROOT_AER_Settings).
//! Read on the server; clients receive the values they need with each session.
[BaseContainerProps(configRoot: true)]
class ROOT_AER_Settings
{
	// Main
	[Attribute("0", desc: "Debug log output", category: "Main")]
	bool m_bDebugMode;

	[Attribute("0", desc: "Testing only: an unknown user name logs in with any password and the account is created", category: "Main")]
	bool m_bDebugAnyLogin;

	[Attribute("0", desc: "Network debug log output", category: "Main")]
	bool m_bNetworkDebug;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Device name when none is set: generic type name or prefab display name", enums: { ParamEnum("Generic", "0"), ParamEnum("Display name", "1") }, category: "Main")]
	int m_iDeviceNameFallback;

	// armaOS
	[Attribute("0", desc: "Allow logging in as root at the terminal. When off, use a user plus sudo / su.", category: "armaOS")]
	bool m_bAllowRootLogin;

	[Attribute("1", desc: "Sudoers act as root on the filesystem without sudo", category: "armaOS")]
	bool m_bCliElevateSudoers;

	[Attribute("0", desc: "Log the account out when a player's session ends", category: "armaOS")]
	bool m_bLogoutOnSessionEnd;

	[Attribute("toor", desc: "Root password of computers without their own", category: "armaOS")]
	string m_sDefaultRootPassword;

	[Attribute("1", desc: "Error sound when a command fails", category: "armaOS")]
	bool m_bEnableErrorSound;

	[Attribute("20480", desc: "Simulated local copy speed in KB/s, 0 = instant", params: "0 102400 1", category: "armaOS")]
	float m_fTransferSpeedLocal;

	[Attribute("2048", desc: "Simulated USB copy speed in KB/s, 0 = instant", params: "0 102400 1", category: "armaOS")]
	float m_fTransferSpeedUsb;

	[Attribute("512", desc: "Simulated network transfer speed in KB/s, 0 = instant", params: "0 102400 1", category: "armaOS")]
	float m_fTransferSpeedNetwork;

	[Attribute("0", desc: "Longest simulated transfer in seconds, 0 = no cap", params: "0 3600 1", category: "armaOS")]
	float m_fTransferMaxDuration;

	[Attribute("0", desc: "Default terminal design index 0-19", params: "0 19 1", category: "armaOS")]
	int m_iTerminalDesign;

	[Attribute("1", desc: "Lines per mouse wheel step", params: "1 3 1", category: "armaOS")]
	int m_iTerminalScrollSpeed;

	[Attribute("0.75", desc: "Terminal font scale", params: "0.5 1 0.05", category: "armaOS")]
	float m_fTerminalDefaultSize;

	[Attribute("1", desc: "Show the screen content on the device model to nearby players", category: "armaOS")]
	bool m_bUiOnTexture;

	[Attribute("2", desc: "Range in m in which players see the screen content", params: "1 10 0.5", category: "armaOS")]
	float m_fUiPlayerRange;

	[Attribute("1", desc: "Screen mirror update interval in s", params: "0.1 2 0.1", category: "armaOS")]
	float m_fUiOnTexUpdateInterval;

	[Attribute("3", desc: "Players that receive one device screen at the same time, 0 = none, -1 = unlimited", params: "-1 50 1", category: "armaOS")]
	int m_iUiMaxConcurrentViewers;

	[Attribute("22", desc: "Lines mirrored to the device screen (screen shows up to 22)", params: "1 22 1", category: "armaOS")]
	int m_iUiMaxTransmitLines;

	[Attribute("0.3", desc: "Interval in s at which the typed input line is mirrored", params: "0 1 0.05", category: "armaOS")]
	float m_fUiKeystrokeSyncInterval;

	[Attribute("1", desc: "Mirror only when the screen content changed", category: "armaOS")]
	bool m_bUiEnableChangeDetection;

	[Attribute("FUBAR(TM) SYSTEMS", desc: "Terminal window title", category: "armaOS")]
	string m_sTerminalDialogTitle;

	[Attribute("FUBAR(TM) BIOS v2.6.0 - Field-Hardened Edition", desc: "Header BIOS line", category: "armaOS")]
	string m_sTerminalBiosVersion;

	[Attribute("(c) 2026 Field-Unified Business Automation & Research Corp. All rights reserved.", desc: "Header copyright line", category: "armaOS")]
	string m_sTerminalCopyright;

	[Attribute("Initializing kernel... please wait...", desc: "Header boot line", category: "armaOS")]
	string m_sTerminalBootMessage;

	[Attribute("> Tip: FUBAR(TM) laptops are drop-tested to military standard. Once.", desc: "Header tip line", category: "armaOS")]
	string m_sTerminalTipMessage;

	[Attribute("1", desc: "Header ASCII art", category: "armaOS")]
	bool m_bTerminalShowAsciiArt;

	[Attribute("FUBAR(TM) Systems - Situation normal since 1987.", desc: "Header tagline", category: "armaOS")]
	string m_sTerminalTagline;

	[Attribute("15", desc: "Boot duration in s", params: "3 15 1", category: "armaOS")]
	float m_fStartupTime;

	[Attribute("15", desc: "Shutdown duration in s", params: "1 15 1", category: "armaOS")]
	float m_fShutdownTime;

	// Desktop
	[Attribute("2", uiwidget: UIWidgets.ComboBox, desc: "Interface of computers without their own mode", enums: { ParamEnum("Terminal only", "0"), ParamEnum("Desktop only", "1"), ParamEnum("Both", "2") }, category: "Desktop")]
	int m_iDesktopDefaultMode;

	[Attribute("1", desc: "File browsing apps in the desktop", category: "Desktop")]
	bool m_bDesktopEnableFileBrowsing;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Default desktop theme", enums: { ParamEnum("Dark", "0"), ParamEnum("Light", "1"), ParamEnum("Olive", "2") }, category: "Desktop")]
	int m_iDesktopDefaultTheme;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Default desktop style of computers without their own style", enums: { ParamEnum("Civilian (Windows)", "0"), ParamEnum("Military (Tactical)", "1") }, category: "Desktop")]
	int m_iDesktopStyle;

	[Attribute("1", desc: "Photos, wallpapers and the Browser may load web addresses (pages, images, videos); each client downloads them", category: "Desktop")]
	bool m_bDesktopWebImages;

	[Attribute("https://wsrv.nl/?output=png&w=1024&encoding=base64&url=%1", desc: "Address that returns a web image as base64 text, %1 = image address (the image address is sent to this service); empty = direct download", category: "Desktop")]
	string m_sDesktopWebImageProxy;

	[Attribute("1", desc: "Map app shows only the computer, its router, routers linked to it and devices connected to it", category: "Desktop")]
	bool m_bDesktopMapSameNetworkOnly;

	// Network
	[Attribute("100", desc: "Default router wireless range in m", params: "1 10000 1", category: "Network")]
	float m_fDefaultWirelessRange;

	[Attribute("1", desc: "Multiplier on every router range", params: "0.1 10 0.1", category: "Network")]
	float m_fWirelessRangeMultiplier;

	// Power
	[Attribute("0.01", desc: "Battery level change that is replicated (fraction)", params: "0 0.2 0.005", category: "Power")]
	float m_fPowerChangeThreshold;

	[Attribute("1", desc: "Replicate battery levels to clients", category: "Power")]
	bool m_bPowerEnableStateSync;

	[Attribute("5", desc: "Power simulation interval in s", params: "1 60 1", category: "Power")]
	float m_fPowerUpdateInterval;

	// Games
	[Attribute("1", desc: "Games enabled", category: "Games")]
	bool m_bGamesEnabled;

	[Attribute("1", desc: "Players may open new game tables", category: "Games")]
	bool m_bGamesAllowHosting;

	[Attribute("0", desc: "Players must share a network to join a table", category: "Games")]
	bool m_bGamesRequireSameNetwork;

	[Attribute("16", desc: "Open tables at the same time", params: "1 100 1", category: "Games")]
	int m_iGamesMaxTables;

	[Attribute("120", desc: "Seconds per turn before the turn is skipped, 0 = off", params: "0 3600 1", category: "Games")]
	float m_fGamesTurnTimeout;

	[Attribute("600", desc: "Seconds without activity before a table closes", params: "60 7200 1", category: "Games")]
	float m_fGamesAbandonTimeout;

	[Attribute("15", desc: "Extra power draw in W while a game is open", params: "0 500 1", category: "Games")]
	float m_fGamesPowerDraw;

	[Attribute("2", desc: "Extra power draw in W while waiting at a table", params: "0 500 1", category: "Games")]
	float m_fGamesIdleDraw;

	[Attribute("1", uiwidget: UIWidgets.ComboBox, desc: "Desktop window size", enums: { ParamEnum("Fullscreen", "0"), ParamEnum("Large", "1"), ParamEnum("Medium", "2"), ParamEnum("Small", "3") }, category: "Desktop")]
	int m_iDesktopSize;

	[Attribute("1", desc: "Table hosts may take extra practice seats and play every side themselves", category: "Games")]
	bool m_bGamesDebugSeats;

	[Attribute("1", desc: "Table hosts may add bot players", category: "Games")]
	bool m_bGamesBots;

	[Attribute("0", desc: "77th JSOC EWO mode: EWO packs broadcast a wireless network and charge carried laptops", category: "EWO")]
	bool m_bEwoMode;

	[Attribute("Radio_ANPRC77", desc: "Prefab names (comma separated, part of the prefab path) of the packs that act as EWO packs", category: "EWO")]
	string m_sEwoPrefabs;

	[Attribute("400", desc: "Energy of an EWO pack, in laptop battery percent", params: "10 5000 10", category: "EWO")]
	float m_fEwoEnergyMax;

	[Attribute("100", desc: "Wireless range of an EWO network in m", params: "5 1000 5", category: "EWO")]
	float m_fEwoWifiRange;

	[Attribute("10", desc: "Range in m to the power source an EWO pack recharges from", params: "2 50 1", category: "EWO")]
	float m_fEwoSourceRange;

	[Attribute("3", desc: "Energy per minute a broadcasting EWO network uses", params: "0 60 0.5", category: "EWO")]
	float m_fEwoWifiDrain;

	[Attribute("10", desc: "Energy per minute an EWO pack delivers to each charging laptop", params: "1 60 1", category: "EWO")]
	float m_fEwoChargeRate;

	[Attribute("5", desc: "Energy per minute an EWO pack takes in from a running power source", params: "1 60 1", category: "EWO")]
	float m_fEwoRechargeRate;

	//------------------------------------------------------------------------------------------------
	//! Text settings by key (Game Master System module). False for an unknown key.
	bool GetText(string key, out string value)
	{
		switch (key)
		{
			case "root_password": value = m_sDefaultRootPassword; return true;
			case "ewo_prefabs": value = m_sEwoPrefabs; return true;
			case "web_image_proxy": value = m_sDesktopWebImageProxy; return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool SetText(string key, string value)
	{
		switch (key)
		{
			case "root_password": m_sDefaultRootPassword = value; return true;
			case "ewo_prefabs": m_sEwoPrefabs = value; return true;
			case "web_image_proxy": m_sDesktopWebImageProxy = value; return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Keys of GetNumber / SetNumber and GetText / SetText.
	static void Keys(notnull array<string> numbers, notnull array<string> texts)
	{
		array<string> n = {
			"allow_root_login", "cli_elevate", "logout_on_end", "debug_any_login", "error_sound", "ui_on_texture",
			"file_browsing", "map_same_network", "games_enabled", "games_hosting", "games_same_network", "games_bots",
			"games_practice", "web_images", "startup_time", "shutdown_time", "transfer_local", "transfer_usb",
			"transfer_network", "range_default", "range_multiplier", "ui_range", "power_interval", "default_mode",
			"default_theme", "desktop_style", "desktop_size", "terminal_design", "ewo_mode", "ewo_energy", "ewo_range", "ewo_source_range",
			"ewo_drain", "ewo_charge", "ewo_recharge"
		};
		array<string> t = {"root_password", "ewo_prefabs", "web_image_proxy"};
		numbers.Copy(n);
		texts.Copy(t);
	}

	//------------------------------------------------------------------------------------------------
	//! Numeric and on/off settings by key (Game Master settings attributes). False for an unknown key.
	bool GetNumber(string key, out float value)
	{
		switch (key)
		{
			case "allow_root_login": value = ROOT_AER_Util.BoolNum(m_bAllowRootLogin); return true;
			case "cli_elevate": value = ROOT_AER_Util.BoolNum(m_bCliElevateSudoers); return true;
			case "logout_on_end": value = ROOT_AER_Util.BoolNum(m_bLogoutOnSessionEnd); return true;
			case "debug_any_login": value = ROOT_AER_Util.BoolNum(m_bDebugAnyLogin); return true;
			case "error_sound": value = ROOT_AER_Util.BoolNum(m_bEnableErrorSound); return true;
			case "ui_on_texture": value = ROOT_AER_Util.BoolNum(m_bUiOnTexture); return true;
			case "file_browsing": value = ROOT_AER_Util.BoolNum(m_bDesktopEnableFileBrowsing); return true;
			case "map_same_network": value = ROOT_AER_Util.BoolNum(m_bDesktopMapSameNetworkOnly); return true;
			case "games_enabled": value = ROOT_AER_Util.BoolNum(m_bGamesEnabled); return true;
			case "games_hosting": value = ROOT_AER_Util.BoolNum(m_bGamesAllowHosting); return true;
			case "games_same_network": value = ROOT_AER_Util.BoolNum(m_bGamesRequireSameNetwork); return true;
			case "games_bots": value = ROOT_AER_Util.BoolNum(m_bGamesBots); return true;
			case "games_practice": value = ROOT_AER_Util.BoolNum(m_bGamesDebugSeats); return true;
			case "web_images": value = ROOT_AER_Util.BoolNum(m_bDesktopWebImages); return true;
			case "startup_time": value = m_fStartupTime; return true;
			case "shutdown_time": value = m_fShutdownTime; return true;
			case "transfer_local": value = m_fTransferSpeedLocal; return true;
			case "transfer_usb": value = m_fTransferSpeedUsb; return true;
			case "transfer_network": value = m_fTransferSpeedNetwork; return true;
			case "range_default": value = m_fDefaultWirelessRange; return true;
			case "range_multiplier": value = m_fWirelessRangeMultiplier; return true;
			case "ui_range": value = m_fUiPlayerRange; return true;
			case "power_interval": value = m_fPowerUpdateInterval; return true;
			case "default_mode": value = m_iDesktopDefaultMode; return true;
			case "default_theme": value = m_iDesktopDefaultTheme; return true;
			case "desktop_style": value = m_iDesktopStyle; return true;
			case "desktop_size": value = m_iDesktopSize; return true;
			case "terminal_design": value = m_iTerminalDesign; return true;
			case "ewo_mode": value = ROOT_AER_Util.BoolNum(m_bEwoMode); return true;
			case "ewo_energy": value = m_fEwoEnergyMax; return true;
			case "ewo_range": value = m_fEwoWifiRange; return true;
			case "ewo_source_range": value = m_fEwoSourceRange; return true;
			case "ewo_drain": value = m_fEwoWifiDrain; return true;
			case "ewo_charge": value = m_fEwoChargeRate; return true;
			case "ewo_recharge": value = m_fEwoRechargeRate; return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool SetNumber(string key, float value)
	{
		switch (key)
		{
			case "allow_root_login": m_bAllowRootLogin = value != 0; return true;
			case "cli_elevate": m_bCliElevateSudoers = value != 0; return true;
			case "logout_on_end": m_bLogoutOnSessionEnd = value != 0; return true;
			case "debug_any_login": m_bDebugAnyLogin = value != 0; return true;
			case "error_sound": m_bEnableErrorSound = value != 0; return true;
			case "ui_on_texture": m_bUiOnTexture = value != 0; return true;
			case "file_browsing": m_bDesktopEnableFileBrowsing = value != 0; return true;
			case "map_same_network": m_bDesktopMapSameNetworkOnly = value != 0; return true;
			case "games_enabled": m_bGamesEnabled = value != 0; return true;
			case "games_hosting": m_bGamesAllowHosting = value != 0; return true;
			case "games_same_network": m_bGamesRequireSameNetwork = value != 0; return true;
			case "games_bots": m_bGamesBots = value != 0; return true;
			case "games_practice": m_bGamesDebugSeats = value != 0; return true;
			case "web_images": m_bDesktopWebImages = value != 0; return true;
			case "startup_time": m_fStartupTime = Math.Clamp(value, 3, 15); return true;
			case "shutdown_time": m_fShutdownTime = Math.Clamp(value, 1, 15); return true;
			case "transfer_local": m_fTransferSpeedLocal = Math.Clamp(value, 0, 102400); return true;
			case "transfer_usb": m_fTransferSpeedUsb = Math.Clamp(value, 0, 102400); return true;
			case "transfer_network": m_fTransferSpeedNetwork = Math.Clamp(value, 0, 102400); return true;
			case "range_default": m_fDefaultWirelessRange = Math.Clamp(value, 1, 10000); return true;
			case "range_multiplier": m_fWirelessRangeMultiplier = Math.Clamp(value, 0.1, 10); return true;
			case "ui_range": m_fUiPlayerRange = Math.Clamp(value, 1, 10); return true;
			case "power_interval": m_fPowerUpdateInterval = Math.Clamp(value, 1, 60); return true;
			case "default_mode": m_iDesktopDefaultMode = Math.ClampInt(Math.Round(value), 0, 2); return true;
			case "default_theme": m_iDesktopDefaultTheme = Math.ClampInt(Math.Round(value), 0, 2); return true;
			case "desktop_style": m_iDesktopStyle = Math.ClampInt(Math.Round(value), 0, 1); return true;
			case "desktop_size": m_iDesktopSize = Math.ClampInt(Math.Round(value), 0, 3); return true;
			case "terminal_design": m_iTerminalDesign = Math.ClampInt(Math.Round(value), 0, 19); return true;
			case "ewo_mode": m_bEwoMode = value != 0; return true;
			case "ewo_energy": m_fEwoEnergyMax = Math.Clamp(value, 10, 5000); return true;
			case "ewo_range": m_fEwoWifiRange = Math.Clamp(value, 5, 1000); return true;
			case "ewo_source_range": m_fEwoSourceRange = Math.Clamp(value, 2, 50); return true;
			case "ewo_drain": m_fEwoWifiDrain = Math.Clamp(value, 0, 60); return true;
			case "ewo_charge": m_fEwoChargeRate = Math.Clamp(value, 1, 60); return true;
			case "ewo_recharge": m_fEwoRechargeRate = Math.Clamp(value, 1, 60); return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Client-relevant values packed for RPC.
	string PackClient()
	{
		array<string> f = {
			m_iTerminalDesign.ToString(), m_iTerminalScrollSpeed.ToString(), m_fTerminalDefaultSize.ToString(),
			m_sTerminalDialogTitle, m_iDesktopDefaultTheme.ToString(), ROOT_AER_Util.BoolStr(m_bDesktopEnableFileBrowsing),
			ROOT_AER_Util.BoolStr(m_bUiOnTexture), m_fUiPlayerRange.ToString(), m_iUiMaxConcurrentViewers.ToString(),
			m_fUiKeystrokeSyncInterval.ToString(), m_iDesktopDefaultMode.ToString(), m_iDeviceNameFallback.ToString(),
			m_iDesktopSize.ToString(), ROOT_AER_Util.BoolStr(m_bDesktopWebImages), ROOT_AER_Util.BoolStr(m_bEwoMode), m_sEwoPrefabs,
			m_sDesktopWebImageProxy
		};
		return ROOT_AER_Codec.JoinSub(f);
	}

	//------------------------------------------------------------------------------------------------
	void UnpackClient(string data)
	{
		array<string> f = ROOT_AER_Codec.SplitSub(data);
		if (f.Count() < 14)
			return;

		m_iTerminalDesign = f[0].ToInt();
		m_iTerminalScrollSpeed = f[1].ToInt();
		m_fTerminalDefaultSize = f[2].ToFloat();
		m_sTerminalDialogTitle = f[3];
		m_iDesktopDefaultTheme = f[4].ToInt();
		m_bDesktopEnableFileBrowsing = f[5] == "1";
		m_bUiOnTexture = f[6] == "1";
		m_fUiPlayerRange = f[7].ToFloat();
		m_iUiMaxConcurrentViewers = f[8].ToInt();
		m_fUiKeystrokeSyncInterval = f[9].ToFloat();
		m_iDesktopDefaultMode = f[10].ToInt();
		m_iDeviceNameFallback = f[11].ToInt();
		m_iDesktopSize = f[12].ToInt();
		m_bDesktopWebImages = f[13] == "1";
		if (f.Count() >= 16)
		{
			m_bEwoMode = f[14] == "1";
			m_sEwoPrefabs = f[15];
		}

		if (f.Count() >= 17)
			m_sDesktopWebImageProxy = f[16];
	}
}
