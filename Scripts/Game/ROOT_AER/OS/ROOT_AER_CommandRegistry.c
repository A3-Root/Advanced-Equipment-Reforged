//! Known armaOS programs by name. Programs reach a computer as files ("#!aer <name>").
class ROOT_AER_CommandRegistry
{
	static const string MARKER = "#!aer ";
	protected static ref ROOT_AER_CommandRegistry s_Instance;
	protected ref map<string, ref ROOT_AER_Command> m_mCommands = new map<string, ref ROOT_AER_Command>();
	protected ref array<string> m_aDefaults = {};
	protected ref array<string> m_aGames = {};

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_CommandRegistry Get()
	{
		if (!s_Instance)
		{
			s_Instance = new ROOT_AER_CommandRegistry();
			s_Instance.RegisterBuiltins();
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterBuiltins()
	{
		array<ref ROOT_AER_Command> defaults = {
			new ROOT_AER_Cmd_Man(), new ROOT_AER_Cmd_Help(), new ROOT_AER_Cmd_Ls(), new ROOT_AER_Cmd_Cd(),
			new ROOT_AER_Cmd_Cat(), new ROOT_AER_Cmd_Date(), new ROOT_AER_Cmd_History(), new ROOT_AER_Cmd_Clear(),
			new ROOT_AER_Cmd_Rm(), new ROOT_AER_Cmd_Mv(), new ROOT_AER_Cmd_Cp(), new ROOT_AER_Cmd_Whoami(),
			new ROOT_AER_Cmd_Mkdir(), new ROOT_AER_Cmd_Touch(), new ROOT_AER_Cmd_Echo(), new ROOT_AER_Cmd_Find(),
			new ROOT_AER_Cmd_Grep(), new ROOT_AER_Cmd_Mount(), new ROOT_AER_Cmd_Umount(), new ROOT_AER_Cmd_Lsusb(),
			new ROOT_AER_Cmd_Chown(), new ROOT_AER_Cmd_Unlock(), new ROOT_AER_Cmd_Sudo(), new ROOT_AER_Cmd_Su(),
			new ROOT_AER_Cmd_Desktop(), new ROOT_AER_Cmd_Ping(), new ROOT_AER_Cmd_Ip("ip"), new ROOT_AER_Cmd_Ip("ifconfig"), new ROOT_AER_Cmd_Wifi(),
			new ROOT_AER_Cmd_Ssh(), new ROOT_AER_Cmd_Msg(), new ROOT_AER_Cmd_Exit(), new ROOT_AER_Cmd_Shutdown(),
			new ROOT_AER_Cmd_Standby(), new ROOT_AER_Cmd_Sshd()
		};

		foreach (ROOT_AER_Command c : defaults)
		{
			Register(c);
			m_aDefaults.Insert(c.GetName());
		}

		Register(new ROOT_AER_Cmd_Crypto());
		Register(new ROOT_AER_Cmd_Crack());

		array<ref ROOT_AER_Command> games = {new ROOT_AER_Cmd_Snake()};
		foreach (string g : ROOT_AER_GameRules.Games())
		{
			games.Insert(new ROOT_AER_Cmd_Game(g));
		}

		foreach (ROOT_AER_Command c : games)
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
	void Register(notnull ROOT_AER_Command command)
	{
		m_mCommands.Set(command.GetName(), command);
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AER_Command Find(string name)
	{
		return m_mCommands.Get(name);
	}

	//------------------------------------------------------------------------------------------------
	//! Installs one program on a computer.
	bool Install(ROOT_AER_ComputerComponent computer, string name)
	{
		ROOT_AER_Command c = computer.FindCommand(name);
		if (!c || !computer.GetFilesystem())
			return false;

		computer.MarkInstalled(name);

		string e;
		string path = c.GetPath();
		ROOT_AER_Filesystem fs = computer.GetFilesystem();
		fs.CreateDir(ROOT_AER_Filesystem.Parent(path), "root", "root", ROOT_AER_FsNode.PERM_PUBLIC_DIR, e, true);
		return fs.WriteFile(path, MARKER + name, "root", false, e, 47, "root");
	}

	//------------------------------------------------------------------------------------------------
	void InstallDefaults(ROOT_AER_ComputerComponent computer)
	{
		foreach (string n : m_aDefaults)
		{
			Install(computer, n);
		}
	}

	//------------------------------------------------------------------------------------------------
	void InstallGames(ROOT_AER_ComputerComponent computer)
	{
		if (!ROOT_AER_SettingsManager.Get().m_bGamesEnabled)
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
