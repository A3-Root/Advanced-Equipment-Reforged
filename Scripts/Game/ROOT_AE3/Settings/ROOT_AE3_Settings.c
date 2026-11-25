//! Mod settings. Defaults below; a mission overrides them through the mission header (m_ROOT_AE3_Settings).
//! Read on the server; clients receive the values they need with each session.
[BaseContainerProps(configRoot: true)]
class ROOT_AE3_Settings
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

	[Attribute("SHITE(TM) COMPUTING", desc: "Terminal window title", category: "armaOS")]
	string m_sTerminalDialogTitle;

	[Attribute("SHITE(TM) BIOS v1.0.0", desc: "Header BIOS line", category: "armaOS")]
	string m_sTerminalBiosVersion;

	[Attribute("(c) 2025 System Hardware Integration & Technology Enterprises", desc: "Header copyright line", category: "armaOS")]
	string m_sTerminalCopyright;

	[Attribute("Initializing kernel... please wait...", desc: "Header boot line", category: "armaOS")]
	string m_sTerminalBootMessage;

	[Attribute("> Tip: SHITE(TM) laptops perform better when plugged in.", desc: "Header tip line", category: "armaOS")]
	string m_sTerminalTipMessage;

	[Attribute("1", desc: "Header ASCII art", category: "armaOS")]
	bool m_bTerminalShowAsciiArt;

	[Attribute("Powered by SHITE(TM) Technologies - Excellence, from the ground up.", desc: "Header tagline", category: "armaOS")]
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

	[Attribute("0", desc: "Map app shows only devices on the same network", category: "Desktop")]
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

	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Desktop window size", enums: { ParamEnum("Fullscreen", "0"), ParamEnum("Large", "1"), ParamEnum("Medium", "2"), ParamEnum("Small", "3") }, category: "Desktop")]
	int m_iDesktopSize;

	[Attribute("0", desc: "One device may take every seat (testing)", category: "Games")]
	bool m_bGamesDebugSeats;

	//------------------------------------------------------------------------------------------------
	//! Client-relevant values packed for RPC.
	string PackClient()
	{
		array<string> f = {
			m_iTerminalDesign.ToString(), m_iTerminalScrollSpeed.ToString(), m_fTerminalDefaultSize.ToString(),
			m_sTerminalDialogTitle, m_iDesktopDefaultTheme.ToString(), ROOT_AE3_Util.BoolStr(m_bDesktopEnableFileBrowsing),
			ROOT_AE3_Util.BoolStr(m_bUiOnTexture), m_fUiPlayerRange.ToString(), m_iUiMaxConcurrentViewers.ToString(),
			m_fUiKeystrokeSyncInterval.ToString(), m_iDesktopDefaultMode.ToString(), m_iDeviceNameFallback.ToString(),
			m_iDesktopSize.ToString()
		};
		return ROOT_AE3_Codec.JoinSub(f);
	}

	//------------------------------------------------------------------------------------------------
	void UnpackClient(string data)
	{
		array<string> f = ROOT_AE3_Codec.SplitSub(data);
		if (f.Count() < 13)
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
	}
}
