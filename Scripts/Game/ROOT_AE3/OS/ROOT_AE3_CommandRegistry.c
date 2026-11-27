//! Known armaOS programs by name. Programs reach a computer as files ("#!ae3 <name>").
class ROOT_AE3_CommandRegistry
{
	static const string MARKER = "#!ae3 ";
	protected static ref ROOT_AE3_CommandRegistry s_Instance;
	protected ref map<string, ref ROOT_AE3_Command> m_mCommands = new map<string, ref ROOT_AE3_Command>();
	protected ref array<string> m_aDefaults = {};
	protected ref array<string> m_aGames = {};

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_CommandRegistry Get()
	{
		if (!s_Instance)
		{
			s_Instance = new ROOT_AE3_CommandRegistry();
			s_Instance.RegisterBuiltins();
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterBuiltins()
	{
		array<ref ROOT_AE3_Command> defaults = {
			new ROOT_AE3_Cmd_Man(), new ROOT_AE3_Cmd_Help(), new ROOT_AE3_Cmd_Ls(), new ROOT_AE3_Cmd_Cd(),
			new ROOT_AE3_Cmd_Cat(), new ROOT_AE3_Cmd_Date(), new ROOT_AE3_Cmd_History(), new ROOT_AE3_Cmd_Clear(),
			new ROOT_AE3_Cmd_Rm(), new ROOT_AE3_Cmd_Mv(), new ROOT_AE3_Cmd_Cp(), new ROOT_AE3_Cmd_Whoami(),
			new ROOT_AE3_Cmd_Mkdir(), new ROOT_AE3_Cmd_Touch(), new ROOT_AE3_Cmd_Echo(), new ROOT_AE3_Cmd_Find(),
			new ROOT_AE3_Cmd_Grep(), new ROOT_AE3_Cmd_Mount(), new ROOT_AE3_Cmd_Umount(), new ROOT_AE3_Cmd_Lsusb(),
			new ROOT_AE3_Cmd_Chown(), new ROOT_AE3_Cmd_Unlock(), new ROOT_AE3_Cmd_Sudo(), new ROOT_AE3_Cmd_Su(),
			new ROOT_AE3_Cmd_Desktop(), new ROOT_AE3_Cmd_Ping(), new ROOT_AE3_Cmd_Ip("ip"), new ROOT_AE3_Cmd_Ip("ifconfig"), new ROOT_AE3_Cmd_Wifi(),
			new ROOT_AE3_Cmd_Ssh(), new ROOT_AE3_Cmd_Msg(), new ROOT_AE3_Cmd_Exit(), new ROOT_AE3_Cmd_Shutdown(),
			new ROOT_AE3_Cmd_Standby(), new ROOT_AE3_Cmd_Sshd()
		};

		foreach (ROOT_AE3_Command c : defaults)
		{
			Register(c);
			m_aDefaults.Insert(c.GetName());
		}

		Register(new ROOT_AE3_Cmd_Crypto());
		Register(new ROOT_AE3_Cmd_Crack());

		array<ref ROOT_AE3_Command> games = {new ROOT_AE3_Cmd_Snake()};
		foreach (string g : ROOT_AE3_GameRules.Games())
		{
			games.Insert(new ROOT_AE3_Cmd_Game(g));
		}

		foreach (ROOT_AE3_Command c : games)
		{
			Register(c);
			m_aGames.Insert(c.GetName());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Drops custom commands of the previous mission (game end).
	static void Reset()
	{
		s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	void Register(notnull ROOT_AE3_Command command)
	{
		m_mCommands.Set(command.GetName(), command);
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_Command Find(string name)
	{
		return m_mCommands.Get(name);
	}

	//------------------------------------------------------------------------------------------------
	//! Installs one program on a computer.
	bool Install(ROOT_AE3_ComputerComponent computer, string name)
	{
		ROOT_AE3_Command c = computer.FindCommand(name);
		if (!c || !computer.GetFilesystem())
			return false;

		computer.MarkInstalled(name);

		string e;
		string path = c.GetPath();
		ROOT_AE3_Filesystem fs = computer.GetFilesystem();
		fs.CreateDir(ROOT_AE3_Filesystem.Parent(path), "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, e, true);
		return fs.WriteFile(path, MARKER + name, "root", false, e, 47, "root");
	}

	//------------------------------------------------------------------------------------------------
	void InstallDefaults(ROOT_AE3_ComputerComponent computer)
	{
		foreach (string n : m_aDefaults)
		{
			Install(computer, n);
		}
	}

	//------------------------------------------------------------------------------------------------
	void InstallGames(ROOT_AE3_ComputerComponent computer)
	{
		if (!ROOT_AE3_SettingsManager.Get().m_bGamesEnabled)
			return;

		foreach (string n : m_aGames)
		{
			Install(computer, n);
		}
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetGameNames()
	{
		return m_aGames;
	}

	//------------------------------------------------------------------------------------------------
	//! Program name of an executable file, empty when the file is no program.
	static string ProgramOf(string content)
	{
		if (!content.StartsWith(MARKER))
			return "";

		string n = content.Substring(MARKER.Length(), content.Length() - MARKER.Length());
		return n.Trim();
	}
}
