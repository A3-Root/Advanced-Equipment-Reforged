//! armaOS shell: login, prompt, tokenizer, redirection, PATH lookup, SSH forwarding, tab completion.
class ROOT_AE3_Shell
{
	protected static ref array<string> s_aPath;

	//------------------------------------------------------------------------------------------------
	//! Program search path.
	static array<string> GetPath()
	{
		if (!s_aPath)
			s_aPath = {"/bin", "/sbin", "/games", "/usr/bin"};

		return s_aPath;
	}

	//------------------------------------------------------------------------------------------------
	static void AddHeader(ROOT_AE3_TerminalState t)
	{
		ROOT_AE3_Settings s = ROOT_AE3_SettingsManager.Get();
		t.AddLine(s.m_sTerminalBiosVersion);
		t.AddLine(s.m_sTerminalCopyright);
		t.AddLine(s.m_sTerminalBootMessage);
		t.AddLine(s.m_sTerminalTipMessage);
		t.AddLine(" ");
		if (s.m_bTerminalShowAsciiArt)
		{
			array<string> art = {
				"  #######    ##              ####### ",
				"#########   ####    Root     ####### ",
				"#########   ####  y0014984   ####### ",
				"#########   #### Wasserstoff ####### ",
				"#########   #### JulesVerner ####### ",
				"#########   ####             ####### ",
				"#################################### ",
				"#################################### ",
				"##############       ############### ",
				"############# $#       ############# ",
				"############# !  73    ############# ",
				"#############         ############## ",
				"###############      ############### ",
				"#################################### ",
				"#################################### ",
				"#  ################################# ",
				"#################################### ",
				" "
			};
			foreach (string l : art)
			{
				t.AddLine(l);
			}

			t.AddLine(s.m_sTerminalTagline);
			t.AddLine(" ");
		}
	}

	//------------------------------------------------------------------------------------------------
	static string PromptOf(ROOT_AE3_ComputerComponent computer, ROOT_AE3_TerminalState t)
	{
		switch (t.m_eInput)
		{
			case ROOT_AE3_ETermInput.LOGIN: return "LOGIN>";
			case ROOT_AE3_ETermInput.PASSWORD: return "PASSWORD>";
			case ROOT_AE3_ETermInput.PROMPT: return t.m_sPromptText;
		}

		if (t.m_App)
			return ">";

		if (t.InSsh())
			return string.Format("%1@%2:%3/>", t.m_sSshUser, t.m_SshTarget.GetHostname(), TrimSlash(t.m_sSshCwd));

		return string.Format("%1@%2:%3/>", t.CurrentUser(), computer.GetHostname(), TrimSlash(t.m_sCwd));
	}

	//------------------------------------------------------------------------------------------------
	protected static string TrimSlash(string p)
	{
		if (p == "/")
			return "";

		return p;
	}

	//------------------------------------------------------------------------------------------------
	//! Appends "[date] message" to a log file of a computer.
	static void WriteLog(ROOT_AE3_ComputerComponent computer, string file, string message)
	{
		if (!computer || !computer.GetFilesystem())
			return;

		computer.GetFilesystem().AppendLog(file, string.Format("[%1] %2\n", ROOT_AE3_Util.Now(), message), 15);
	}

	//------------------------------------------------------------------------------------------------
	//! A typed line, routed by the terminal input mode.
	static void OnLine(ROOT_AE3_Session session, string line)
	{
		ROOT_AE3_TerminalState t = session.GetTerminal();
		ROOT_AE3_ComputerComponent computer = session.m_Computer;
		if (t.m_bBusy)
			return;

		switch (t.m_eInput)
		{
			case ROOT_AE3_ETermInput.LOGIN:
			{
				session.Out("LOGIN> " + line);
				string name = line.Trim();
				if (name.IsEmpty())
					break;

				bool rootBlocked = name == "root" && !computer.AllowsRootLogin();
				if (!rootBlocked)
				{
					t.m_sLoginName = name;
					t.m_eInput = ROOT_AE3_ETermInput.PASSWORD;
				}
				else
				{
					session.Out("Login failed.");
					session.Out("Direct root login is disabled. Log in as a user and use sudo or su.");
					WriteLog(computer, "/var/log/auth.log", string.Format("login: blocked user '%1'", name));
				}

				break;
			}

			case ROOT_AE3_ETermInput.PASSWORD:
			{
				session.Out("PASSWORD> " + ROOT_AE3_Util.Repeat("*", line.Length()));
				string user = t.m_sLoginName;
				bool debugAny = ROOT_AE3_SettingsManager.Get().m_bDebugAnyLogin && !computer.HasUser(user) && ROOT_AE3_Filesystem.ValidName(user);
				if (debugAny)
					computer.AddUser(user, line);

				if ((debugAny && computer.HasUser(user)) || session.CheckAuth(computer.HasUser(user) && computer.CheckPassword(user, line)))
				{
					Login(session, user);
				}
				else
				{
					session.Out("Login failed.");
					WriteLog(computer, "/var/log/auth.log", string.Format("login: wrong password for '%1'", user));
					t.m_eInput = ROOT_AE3_ETermInput.LOGIN;
					ROOT_AE3_Sound.Play(computer.GetOwner(), ROOT_AE3_Sound.ERROR);
				}

				break;
			}

			case ROOT_AE3_ETermInput.PROMPT:
			{
				string shown = line;
				if (t.m_bPromptHidden)
					shown = ROOT_AE3_Util.Repeat("*", line.Length());

				session.Out(t.m_sPromptText + " " + shown);
				ROOT_AE3_PromptHandler h = t.m_Prompt;
				t.m_Prompt = null;
				t.m_eInput = ROOT_AE3_ETermInput.SHELL;
				t.m_bPromptHidden = false;
				ROOT_AE3_ShellContext ctx = NewContext(session);
				if (h && !h.OnInput(ctx, line))
					return;

				ctx.Finish();
				return;
			}

			default:
			{
				if (t.m_App)
				{
					session.Out("> " + line);
					if (!t.m_App.OnLine(session, line))
					{
						ROOT_AE3_InteractiveApp app = t.m_App;
						t.m_App = null;
						app.OnExit(session);
					}

					break;
				}

				session.Out(PromptOf(computer, t) + MaskSecrets(line));
				Process(session, line);
				return;
			}
		}

		session.SendPrompt();
	}

	//------------------------------------------------------------------------------------------------
	static void Login(ROOT_AE3_Session session, string user)
	{
		ROOT_AE3_TerminalState t = session.GetTerminal();
		ROOT_AE3_ComputerComponent computer = session.m_Computer;
		ClearRemote(t);
		t.m_sUser = user;
		t.m_sLoginName = "";
		t.m_aSuUsers.Clear();
		t.m_aSuCwds.Clear();
		t.m_eInput = ROOT_AE3_ETermInput.SHELL;
		string home = ROOT_AE3_Filesystem.HomeOf(user);
		if (computer.GetFilesystem().Exists(home))
			t.m_sCwd = home;
		else
			t.m_sCwd = "/";

		WriteLog(computer, "/var/log/auth.log", string.Format("login: user '%1' logged in", user));
		session.Out(string.Format("Welcome %1. Type 'help' for a list of programs.", user));
		ROOT_AE3_API.InvokeLogin(computer, user);
	}

	//------------------------------------------------------------------------------------------------
	//! Logs out (exit at the top level).
	static void Logout(ROOT_AE3_Session session)
	{
		ROOT_AE3_TerminalState t = session.GetTerminal();
		WriteLog(session.m_Computer, "/var/log/auth.log", string.Format("logout: user '%1'", t.m_sUser));
		ClearRemote(t);
		t.m_sUser = "";
		t.m_aSuUsers.Clear();
		t.m_aSuCwds.Clear();
		t.m_eInput = ROOT_AE3_ETermInput.LOGIN;
		t.m_sCwd = "/";
		session.Clear();
		AddHeader(t);
		session.Out(t.Tail(ROOT_AE3_TerminalState.MAX_LINES));
		t.m_aBuffer.Resize(t.m_aBuffer.Count() / 2);
	}

	//------------------------------------------------------------------------------------------------
	//! Command line with password arguments of ssh, unlock and wifi connect replaced by '*'.
	static string MaskSecrets(string line)
	{
		int redir = line.IndexOf(">");
		if (redir > 0)
			return MaskSecrets(line.Substring(0, redir).Trim()) + " " + line.Substring(redir, line.Length() - redir);

		array<string> p = {};
		line.Split(" ", p, false);
		int o = 0;
		if (!p.IsEmpty() && p[0] == "sudo")
			o = 1;

		int idx = -1;
		if (p.Count() >= 3 + o && (p[o] == "ssh" || p[o] == "unlock"))
			idx = p.Count() - 1;
		else if (p.Count() >= 4 + o && p[o] == "wifi" && p[o + 1] == "connect")
			idx = 3 + o;

		if (p.Count() > o && p[o] == "crypto")
		{
			for (int k = o; k + 1 < p.Count(); k++)
			{
				if (p[k] == "-k")
					idx = k + 1;
			}
		}

		if (idx < 0)
			return line;

		p[idx] = ROOT_AE3_Util.Repeat("*", p[idx].Length());
		string res;
		foreach (int i, string w : p)
		{
			if (i > 0)
				res += " ";

			res += w;
		}

		return res;
	}

	//------------------------------------------------------------------------------------------------
	//! Drops SSH connection, running program and pending prompt (login / logout).
	protected static void ClearRemote(ROOT_AE3_TerminalState t)
	{
		t.m_SshTarget = null;
		t.m_sSshUser = "";
		t.m_sSshCwd = "/";
		t.m_App = null;
		t.m_Prompt = null;
		t.m_bBusy = false;
		t.m_iGeneration++;
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_ShellContext NewContext(ROOT_AE3_Session session)
	{
		ROOT_AE3_TerminalState t = session.GetTerminal();
		ROOT_AE3_ComputerComponent exec = session.m_Computer;
		string user = t.CurrentUser();
		if (t.InSsh())
		{
			exec = t.m_SshTarget;
			user = t.m_sSshUser;
		}

		return new ROOT_AE3_ShellContext(session, exec, user);
	}

	//------------------------------------------------------------------------------------------------
	//! Quote-aware tokenizer. Double and single quotes group, backslash escapes the next char.
	static array<string> Tokenize(string line)
	{
		array<string> tokens = {};
		string cur;
		bool inToken;
		string quote;
		int n = line.Length();
		for (int i = 0; i < n; i++)
		{
			string c = line.Get(i);
			if (!quote.IsEmpty())
			{
				if (c == quote)
				{
					quote = "";
					continue;
				}

				if (c == "\\" && quote == "\"" && i + 1 < n)
				{
					i++;
					cur += line.Get(i);
					continue;
				}

				cur += c;
				continue;
			}

			if (c == "\"" || c == "'")
			{
				quote = c;
				inToken = true;
				continue;
			}

			if (c == "\\" && i + 1 < n)
			{
				i++;
				cur += line.Get(i);
				inToken = true;
				continue;
			}

			if (c == " " || c == "\t")
			{
				if (inToken)
				{
					tokens.Insert(cur);
					cur = "";
					inToken = false;
				}

				continue;
			}

			cur += c;
			inToken = true;
		}

		if (inToken)
			tokens.Insert(cur);

		return tokens;
	}

	//------------------------------------------------------------------------------------------------
	//! Splits "cmd args > file" / ">> file" outside quotes. Returns the command part.
	static string ParseRedirect(string line, out string target, out bool append, out bool hasRedirect)
	{
		string quote;
		for (int i = 0; i < line.Length(); i++)
		{
			string c = line.Get(i);
			if (!quote.IsEmpty())
			{
				if (c == quote)
					quote = "";

				continue;
			}

			if (c == "\"" || c == "'")
			{
				quote = c;
				continue;
			}

			if (c == ">")
			{
				hasRedirect = true;
				int start = i + 1;
				if (start < line.Length() && line.Get(start) == ">")
				{
					append = true;
					start++;
				}

				array<string> t = Tokenize(line.Substring(start, line.Length() - start));
				if (!t.IsEmpty())
					target = t[0];

				return line.Substring(0, i);
			}
		}

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! Executes one command line in the session.
	static void Process(ROOT_AE3_Session session, string line)
	{
		ROOT_AE3_TerminalState t = session.GetTerminal();
		s_iScriptDepth = 0;
		ROOT_AE3_ShellContext ctx = NewContext(session);
		string target;
		bool append, hasRedirect;
		string cmdLine = ParseRedirect(line, target, append, hasRedirect);
		array<string> tokens = Tokenize(cmdLine);
		if (tokens.IsEmpty())
		{
			session.SendPrompt();
			return;
		}

		session.m_Computer.AddHistory(t.CurrentUser(), MaskSecrets(line));
		ROOT_AE3_Regex.SetOwner(session.m_iPlayerId);
		string name = tokens[0];
		tokens.RemoveOrdered(0);

		if (t.InSsh())
		{
			if (name == "exit")
			{
				if (!t.m_aSshSu.IsEmpty())
				{
					int last = t.m_aSshSu.Count() - 1;
					t.m_sSshUser = t.m_aSshSu[last];
					t.m_sSshCwd = t.m_aSshSuCwd[last];
					t.m_aSshSu.Remove(last);
					t.m_aSshSuCwd.Remove(last);
				}
				else
				{
					EndSsh(session, "Connection closed.");
				}

				ctx.Finish();
				return;
			}

			if (!SshAlive(session))
			{
				EndSsh(session, "Connection lost.");
				ctx.Finish();
				return;
			}
		}

		if (hasRedirect)
		{
			if (target.IsEmpty())
			{
				ctx.Err("Redirect without target file.");
				ctx.Finish();
				return;
			}

			ctx.m_aCapture = {};
			ctx.m_sRedirectTarget = target;
			ctx.m_bRedirectAppend = append;
		}

		Execute(ctx, name, tokens);
		if (!ctx.IsHeld())
			ctx.Finish();
	}

	//------------------------------------------------------------------------------------------------
	//! Program name or path resolves to a file.
	protected static bool ProgramExists(ROOT_AE3_ShellContext ctx, string name)
	{
		string err;
		if (name.Contains("/"))
			return ctx.Fs().Resolve(ctx.Abs(name), ctx.FsUser(), err) != null;

		foreach (string dir : GetPath())
		{
			if (ctx.Fs().Resolve(dir + "/" + name, "root", err))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Resolves a program by name or path on the executing computer and runs it.
	static void Execute(ROOT_AE3_ShellContext ctx, string name, array<string> args)
	{
		string err;
		ROOT_AE3_FsNode file;
		ROOT_AE3_Filesystem fs = ctx.Fs();
		string fsUser = ctx.FsUser();
		if (name.Contains("/"))
		{
			file = fs.Resolve(ctx.Abs(name), fsUser, err);
		}
		else
		{
			foreach (string dir : GetPath())
			{
				file = fs.Resolve(dir + "/" + name, "root", err);
				if (file)
					break;
			}
		}

		if (!file || file.m_bDir)
		{
			ctx.Err(name + ": command not found");
			return;
		}

		if (!file.HasPermission(fsUser, 2))
		{
			ctx.Err(name + ": " + ROOT_AE3_Filesystem.ERR_PERMISSION);
			return;
		}

		string program = ROOT_AE3_CommandRegistry.ProgramOf(file.m_sContent);
		bool everyoneWrites = (file.m_iPerms & 16) != 0;
		bool ownFs = fs.GetLastFs() == fs;
		bool inPath = !name.Contains("/") || GetPath().Contains(ROOT_AE3_Filesystem.Parent(ctx.Abs(name)));
		if (file.m_sOwner != "root" || everyoneWrites || !ownFs || !inPath || !ctx.m_Exec.IsInstalled(program))
			program = "";

		if (program.IsEmpty())
		{
			if (ROOT_AE3_Intel.IsLocked(file.m_sContent) || ROOT_AE3_Intel.IsMedia(file.m_sContent) || file.m_sContent.StartsWith(ROOT_AE3_CommandRegistry.MARKER))
			{
				ctx.Err(name + ": cannot execute this file");
				return;
			}

			RunScript(ctx, file.m_sContent);
			return;
		}

		ROOT_AE3_Command cmd = ctx.m_Exec.FindCommand(program);
		if (!cmd)
		{
			ctx.Err(name + ": program not loaded (custom commands are added again by the mission after a restore)");
			return;
		}

		if (ctx.m_Term.InSsh() && !cmd.IsSshCompatible())
		{
			ctx.Err(string.Format("%1: not available in an SSH session", program));
			return;
		}

		if (args.Count() == 1 && args[0] == "--help")
		{
			ctx.Out(cmd.GetManual());
			return;
		}

		ctx.m_sCommand = program;
		ROOT_AE3_API.InvokeCommand(ctx.m_Exec, ctx.m_sUser, program, args);
		cmd.Run(ctx, args);
	}

	//------------------------------------------------------------------------------------------------
	//! Executable text file: one command per line, lines starting with # are comments.
	protected static int s_iScriptDepth;
	protected static int s_iScriptBudget;
	static const int MAX_SCRIPT_DEPTH = 8;
	static const int MAX_SCRIPT_LINES = 50;	//!< Lines executed per top-level script run

	protected static void RunScript(ROOT_AE3_ShellContext ctx, string content)
	{
		if (s_iScriptDepth == 0)
			s_iScriptBudget = MAX_SCRIPT_LINES;

		if (s_iScriptDepth >= MAX_SCRIPT_DEPTH)
		{
			ctx.Err("script nesting too deep");
			return;
		}

		s_iScriptDepth++;
		array<string> lines = {};
		content.Split("\n", lines, true);
		int count;
		foreach (string l : lines)
		{
			l = l.Trim();
			if (l.IsEmpty() || l.StartsWith("#"))
				continue;

			count++;
			s_iScriptBudget--;
			if (count > 50 || s_iScriptBudget < 0)
			{
				if (s_iScriptBudget < 0)
					ctx.Err("script line limit reached");

				break;
			}

			array<string> tokens = Tokenize(l);
			if (tokens.IsEmpty())
				continue;

			string name = tokens[0];
			tokens.RemoveOrdered(0);
			if (!ProgramExists(ctx, name))
			{
				ctx.Err(string.Format("script line %1: command not found", count));
				continue;
			}

			Execute(ctx, name, tokens);
			if (ctx.IsHeld())
				break;
		}

		s_iScriptDepth--;
	}

	//------------------------------------------------------------------------------------------------
	//! True while the SSH peer still runs and is reachable.
	static bool SshAlive(ROOT_AE3_Session session)
	{
		ROOT_AE3_TerminalState t = session.GetTerminal();
		if (!t.m_SshTarget || !t.m_SshTarget.IsRunning())
			return false;

		ROOT_AE3_NetworkComponent a = session.m_Computer.GetNetwork();
		ROOT_AE3_NetworkComponent b = t.m_SshTarget.GetNetwork();
		int hops;
		string reason;
		return ROOT_AE3_NetworkSystem.Get().CanReach(a, b, hops) && ROOT_AE3_NetworkSystem.Get().SshAllowed(a, b, reason);
	}

	//------------------------------------------------------------------------------------------------
	static void EndSsh(ROOT_AE3_Session session, string message)
	{
		ROOT_AE3_TerminalState t = session.GetTerminal();
		if (t.m_SshTarget)
			WriteLog(t.m_SshTarget, "/var/log/auth.log", string.Format("ssh: session of '%1' closed", t.m_sSshUser));

		t.m_SshTarget = null;
		t.m_sSshUser = "";
		t.m_sSshCwd = "/";
		t.m_aSshSu.Clear();
		t.m_aSshSuCwd.Clear();
		session.Out(message);
	}

	//------------------------------------------------------------------------------------------------
	//! Ctrl+C: cancels a pending prompt or running program.
	static void Interrupt(ROOT_AE3_Session session)
	{
		ROOT_AE3_TerminalState t = session.GetTerminal();
		if (t.m_App)
		{
			ROOT_AE3_InteractiveApp app = t.m_App;
			t.m_App = null;
			app.OnExit(session);
		}

		if (t.m_eInput == ROOT_AE3_ETermInput.PROMPT)
		{
			t.m_Prompt = null;
			t.m_eInput = ROOT_AE3_ETermInput.SHELL;
		}

		t.m_bBusy = false;
		t.m_iGeneration++;
		session.Out("^C");
		session.SendPrompt();
	}

	//------------------------------------------------------------------------------------------------
	//! Tab completion of the last word: program names for the first word, paths otherwise.
	static string Complete(ROOT_AE3_Session session, string line)
	{
		ROOT_AE3_TerminalState t = session.GetTerminal();
		if (t.m_eInput != ROOT_AE3_ETermInput.SHELL || t.m_App)
			return line;

		ROOT_AE3_ShellContext ctx = NewContext(session);
		int sp = line.LastIndexOf(" ");
		string head, word;
		if (sp >= 0)
		{
			head = line.Substring(0, sp + 1);
			word = line.Substring(sp + 1, line.Length() - sp - 1);
		}
		else
		{
			word = line;
		}

		array<string> candidates = {};
		string dirPart, prefix, err;
		ROOT_AE3_Filesystem fs = ctx.Fs();
		if (sp < 0 && !word.Contains("/"))
		{
			foreach (string dir : GetPath())
			{
				ROOT_AE3_FsNode d = fs.Resolve(dir, "root", err);
				if (!d || !d.m_bDir)
					continue;

				foreach (string n : d.GetSortedNames())
				{
					if (n.StartsWith(word) && !candidates.Contains(n))
						candidates.Insert(n);
				}
			}

			prefix = "";
		}
		else
		{
			int slash = word.LastIndexOf("/");
			string base = word;
			if (slash >= 0)
			{
				dirPart = word.Substring(0, slash + 1);
				base = word.Substring(slash + 1, word.Length() - slash - 1);
			}

			string lookup = dirPart;
			if (lookup.IsEmpty())
				lookup = ".";

			ROOT_AE3_FsNode d = fs.Resolve(ctx.Abs(lookup), ctx.FsUser(), err);
			if (d && d.m_bDir && d.HasPermission(ctx.FsUser(), 0))
			{
				foreach (string n : d.GetSortedNames())
				{
					if (n.StartsWith(base))
					{
						string cand = n;
						if (d.GetChild(n).m_bDir)
							cand += "/";

						candidates.Insert(cand);
					}
				}
			}

			prefix = dirPart;
		}

		if (candidates.IsEmpty())
			return line;

		if (candidates.Count() == 1)
		{
			string done = head + prefix + candidates[0];
			if (!done.EndsWith("/"))
				done += " ";

			return done;
		}

		string common = candidates[0];
		foreach (string c : candidates)
		{
			while (!c.StartsWith(common))
			{
				common = common.Substring(0, common.Length() - 1);
			}
		}

		session.Out(PromptOf(session.m_Computer, t) + line);
		session.Out(ROOT_AE3_Util.JoinFrom(candidates, 0, "  "));
		session.SendPrompt();
		return head + prefix + common;
	}
}
