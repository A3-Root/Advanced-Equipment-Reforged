//! Virtual Unix filesystem with owner / everyone permissions, symlinks, mount points and trash.
//! Server only. Errors are returned through the out string; on success it stays empty.
class ROOT_AE3_Filesystem : Managed
{
	static const string ERR_NOT_FOUND = "No such file or directory";
	static const string ERR_PERMISSION = "Permission denied";
	static const string ERR_NOT_DIR = "Not a directory";
	static const string ERR_IS_DIR = "Is a directory";
	static const string ERR_EXISTS = "File exists";
	static const string ERR_NOT_EMPTY = "Directory not empty";
	static const string ERR_BAD_PATH = "Invalid path";
	static const int MAX_LINK_DEPTH = 8;

	ref ROOT_AE3_FsNode m_Root;
	ROOT_AE3_ComputerComponent m_Computer;	//!< Owner computer for the file change event (null for flash drives)
	float m_fCapacityMB;				//!< Virtual size limit in MB; 0 = none
	protected ROOT_AE3_Filesystem m_LastFs;	//!< Filesystem holding the node of the last Resolve (mounts)
	protected string m_sLastUnmounted;		//!< Empty mount point passed by the last Resolve
	static const int MAX_CONTENT = 500000;	//!< Stored characters per filesystem
	static const int MAX_LOG = 16000;		//!< Characters kept per log file (AppendLog)
	static const int MAX_FILE = 65536;		//!< Characters per file
	static const int MAX_NODES = 5000;		//!< Files, folders and links per filesystem
	protected float m_fTotalsTime = -1;
	protected int m_iTotalChars;
	protected int m_iTotalSize;
	protected int m_iTotalNodes;
	protected ref map<string, ROOT_AE3_Filesystem> m_mMounts = new map<string, ROOT_AE3_Filesystem>();
	ref map<string, string> m_mTrashMeta = new map<string, string>();	//!< Trash entry name -> original path.

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_Filesystem()
	{
		m_Root = new ROOT_AE3_FsNode("", true, "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates the default tree of an armaOS computer.
	void SeedDefaults()
	{
		string e;
		CreateDir("/tmp", "root", "root", 63, e, true);
		CreateDir("/mnt", "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, e, true);
		CreateDir("/var/log", "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, e, true);
		CreateDir("/var/mail", "root", "root", 63, e, true);
		CreateDir("/var/sent", "root", "root", 63, e, true);
		CreateDir("/var/chat", "root", "root", 63, e, true);
		CreateDir("/etc", "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, e, true);
		CreateDir("/home", "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, e, true);
		CreateDir("/bin", "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, e, true);
		CreateDir("/sbin", "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, e, true);
		CreateDir("/games", "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, e, true);
		CreateDir("/root", "root", "root", 7, e, true);
		CreateDir("/sys/battery", "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, e, true);
		CreateDir("/.trash", "root", "root", 7, e, true);
		WriteFile("/var/log/syslog", "", "root", false, e, 15);
		WriteFile("/var/log/auth.log", "", "root", false, e, 15);
		WriteFile("/var/log/browser_history", "", "root", false, e, 15);
		WriteFile("/etc/sudoers", "", "root", false, e, 3);
		WriteFile("/sys/battery/capacity", "", "root", false, e, 15);
	}

	//------------------------------------------------------------------------------------------------
	void Mount(string id, ROOT_AE3_Filesystem fs)
	{
		if (fs)
			m_mMounts.Set(id, fs);
		else
			m_mMounts.Remove(id);
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_Filesystem GetMount(string id)
	{
		return m_mMounts.Get(id);
	}

	//------------------------------------------------------------------------------------------------
	//! Mount point id of an empty mount point the last Resolve passed through (no drive mounted), else "".
	string GetLastUnmounted()
	{
		return m_sLastUnmounted;
	}

	//------------------------------------------------------------------------------------------------
	//! Error text for writes into an empty mount point.
	static string NoDrive(string id)
	{
		return "No drive in " + id;
	}

	//------------------------------------------------------------------------------------------------
	static string HomeOf(string user)
	{
		if (user == "root")
			return "/root";

		return "/home/" + user;
	}

	//------------------------------------------------------------------------------------------------
	//! Absolute, normalized path. Handles ".", "..", "~" and relative paths.
	static string Normalize(string cwd, string path, string user)
	{
		path = path.Trim();
		if (path.StartsWith("~"))
			path = HomeOf(user) + path.Substring(1, path.Length() - 1);

		if (!path.StartsWith("/"))
			path = cwd + "/" + path;

		array<string> raw = {};
		path.Split("/", raw, true);
		array<string> parts = {};
		foreach (string p : raw)
		{
			if (p == ".")
				continue;

			if (p == "..")
			{
				if (!parts.IsEmpty())
					parts.Remove(parts.Count() - 1);

				continue;
			}

			parts.Insert(p);
		}

		if (parts.IsEmpty())
			return "/";

		string r;
		foreach (string p : parts)
		{
			r += "/" + p;
		}

		return r;
	}

	//------------------------------------------------------------------------------------------------
	static string Parent(string abs)
	{
		int i = abs.LastIndexOf("/");
		if (i <= 0)
			return "/";

		return abs.Substring(0, i);
	}

	//------------------------------------------------------------------------------------------------
	static string BaseName(string abs)
	{
		int i = abs.LastIndexOf("/");
		if (i < 0)
			return abs;

		return abs.Substring(i + 1, abs.Length() - i - 1);
	}

	//------------------------------------------------------------------------------------------------
	static bool ValidName(string name)
	{
		if (name.IsEmpty() || name.Length() > 64 || name == "." || name == ".." || name.Contains("/"))
			return false;

		for (int i = 0, n = name.Length(); i < n; i++)
		{
			if (name.Get(i).ToAscii() < 32)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Node at an absolute path. Traversal needs execute permission on every directory.
	ROOT_AE3_FsNode Resolve(string abs, string user, out string err, bool followLast = true)
	{
		return ResolveDepth(abs, user, err, followLast, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_FsNode ResolveDepth(string abs, string user, out string err, bool followLast, int depth)
	{
		err = "";
		if (depth > MAX_LINK_DEPTH)
		{
			err = "Too many levels of symbolic links";
			return null;
		}

		array<string> parts = {};
		abs.Split("/", parts, true);
		ROOT_AE3_FsNode cur = m_Root;
		ROOT_AE3_Filesystem curFs = this;
		string walked;
		if (depth == 0)
			m_sLastUnmounted = "";

		for (int i = 0; i < parts.Count(); i++)
		{
			if (!cur.m_bDir)
			{
				err = ERR_NOT_DIR;
				return null;
			}

			if (!cur.HasPermission(user, 2))
			{
				err = ERR_PERMISSION;
				return null;
			}

			ROOT_AE3_FsNode next = cur.GetChild(parts[i]);
			if (!next)
			{
				err = ERR_NOT_FOUND;
				return null;
			}

			walked += "/" + parts[i];
			bool last = i == parts.Count() - 1;
			if (next.IsLink() && (!last || followLast))
			{
				string target = Normalize(Parent(walked), next.m_sLink, user);
				string rest;
				for (int k = i + 1; k < parts.Count(); k++)
				{
					rest += "/" + parts[k];
				}

				return ResolveDepth(target + rest, user, err, followLast, depth + 1);
			}

			if (!next.m_sMountId.IsEmpty())
			{
				ROOT_AE3_Filesystem mounted = curFs.GetMount(next.m_sMountId);
				if (mounted)
				{
					curFs = mounted;
					next = mounted.m_Root;
				}
				else
				{
					m_sLastUnmounted = next.m_sMountId;
				}
			}

			cur = next;
		}

		m_LastFs = curFs;
		return cur;
	}

	//------------------------------------------------------------------------------------------------
	//! File change event for writes done outside the filesystem methods.
	void NotifyChanged(string abs)
	{
		Changed(abs);
	}

	//------------------------------------------------------------------------------------------------
	//! File change event for scripts (ROOT_AE3_API.GetOnFileChanged).
	protected void Changed(string abs)
	{
		if (m_Computer)
			ROOT_AE3_API.InvokeFileChanged(m_Computer, abs);
	}

	//------------------------------------------------------------------------------------------------
	//! Filesystem that holds the node found by the last Resolve (this one or a mounted drive).
	ROOT_AE3_Filesystem GetLastFs()
	{
		if (m_LastFs)
			return m_LastFs;

		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! Space check for adding node (or extra characters when node is null).
	bool Fits(ROOT_AE3_FsNode node, out string err, int extraChars = 0, int newNodes = 0)
	{
		int chars = extraChars;
		int size = extraChars;
		int nodes = newNodes;
		if (node)
		{
			chars += node.GetStoredChars();
			size += node.GetSize();
			nodes += node.CountNodes();
		}

		// Totals are walked once per frame and counted up for writes granted in that frame.
		float now = GetGame().GetWorld().GetWorldTime();
		if (now != m_fTotalsTime)
		{
			m_fTotalsTime = now;
			m_iTotalChars = m_Root.GetStoredChars();
			m_iTotalSize = m_Root.GetSize();
			m_iTotalNodes = m_Root.CountNodes();
		}

		if (m_iTotalChars + chars > MAX_CONTENT || m_iTotalNodes + nodes > MAX_NODES || (m_fCapacityMB > 0 && (m_iTotalSize + size) / 1048576.0 > m_fCapacityMB))
		{
			err = "No space left on device";
			return false;
		}

		m_iTotalChars += chars;
		m_iTotalSize += size;
		m_iTotalNodes += nodes;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Parent directory node of abs with write permission check.
	protected ROOT_AE3_FsNode WritableParent(string abs, string user, out string err)
	{
		ROOT_AE3_FsNode dir = Resolve(Parent(abs), user, err);
		if (!dir)
			return null;

		if (!m_sLastUnmounted.IsEmpty())
		{
			err = NoDrive(m_sLastUnmounted);
			return null;
		}

		if (!dir.m_bDir)
		{
			err = ERR_NOT_DIR;
			return null;
		}

		if (!dir.HasPermission(user, 1))
		{
			err = ERR_PERMISSION;
			return null;
		}

		return dir;
	}

	//------------------------------------------------------------------------------------------------
	bool Exists(string abs)
	{
		string e;
		return Resolve(abs, "root", e) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a directory. parents = create missing parents (mkdir -p).
	bool CreateDir(string abs, string user, string owner, int perms, out string err, bool parents = false)
	{
		err = "";
		if (abs == "/")
			return true;

		ROOT_AE3_FsNode existing = Resolve(abs, user, err);
		if (existing)
		{
			if (existing.m_bDir && parents)
			{
				err = "";
				return true;
			}

			err = ERR_EXISTS;
			return false;
		}

		if (parents && !Exists(Parent(abs)))
		{
			if (!CreateDir(Parent(abs), user, owner, perms, err, true))
				return false;
		}

		string name = BaseName(abs);
		if (!ValidName(name))
		{
			err = ERR_BAD_PATH;
			return false;
		}

		ROOT_AE3_FsNode dir = WritableParent(abs, user, err);
		if (!dir || !GetLastFs().Fits(null, err, name.Length(), 1))
			return false;

		dir.SetChild(new ROOT_AE3_FsNode(name, true, owner, perms));
		err = "";
		Changed(abs);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Writes or creates a file. perms is used only when the file is created.
	bool WriteFile(string abs, string content, string user, bool append, out string err, int perms = -1, string owner = "", bool noLink = false)
	{
		err = "";
		if (noLink)
		{
			ROOT_AE3_FsNode last = Resolve(abs, user, err, false);
			if (last && last.IsLink())
			{
				err = "Refusing to write through a symbolic link";
				return false;
			}

			err = "";
		}

		content = ROOT_AE3_Codec.Clean(content);
		ROOT_AE3_FsNode node = Resolve(abs, user, err);
		if (node)
		{
			if (node.m_bDir)
			{
				err = ERR_IS_DIR;
				return false;
			}

			if (!node.HasPermission(user, 1))
			{
				err = ERR_PERMISSION;
				return false;
			}

			int total = content.Length();
			if (append)
				total += node.m_sContent.Length();

			if (total > MAX_FILE)
			{
				err = "File too large";
				return false;
			}

			int grow = content.Length();
			if (!append)
				grow -= node.m_sContent.Length();

			if (grow > 0 && !GetLastFs().Fits(null, err, grow))
				return false;

			if (append)
				node.m_sContent += content;
			else
				node.m_sContent = content;

			if (!noLink)
				Changed(abs);

			return true;
		}

		if (err != ERR_NOT_FOUND)
			return false;

		string name = BaseName(abs);
		if (!ValidName(name))
		{
			err = ERR_BAD_PATH;
			return false;
		}

		ROOT_AE3_FsNode dir = WritableParent(abs, user, err);
		if (!dir)
			return false;

		if (owner.IsEmpty())
			owner = user;

		if (perms < 0)
			perms = ROOT_AE3_FsNode.PERM_FILE_DEFAULT;

		if (content.Length() > MAX_FILE)
		{
			err = "File too large";
			return false;
		}

		if (!GetLastFs().Fits(null, err, content.Length() + name.Length(), 1))
			return false;

		ROOT_AE3_FsNode f = new ROOT_AE3_FsNode(name, false, owner, perms);
		f.m_sContent = content;
		dir.SetChild(f);
		if (!noLink)
			Changed(abs);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Root append to a log file; keeps the newest lines within MAX_LOG characters.
	void AppendLog(string abs, string line, int perms)
	{
		string err;
		if (!WriteFile(abs, line, "root", true, err, perms, "root", true))
			return;

		ROOT_AE3_FsNode node = Resolve(abs, "root", err, false);
		if (!node || node.m_bDir || node.IsLink() || node.m_sContent.Length() <= MAX_LOG)
			return;

		string c = node.m_sContent;
		int cut = c.Length() - MAX_LOG / 2;
		int nl = c.IndexOfFrom(cut, "\n");
		if (nl < 0)
			nl = cut;

		node.m_sContent = c.Substring(nl + 1, c.Length() - nl - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! File content with read permission check.
	bool ReadFile(string abs, string user, out string content, out string err)
	{
		ROOT_AE3_FsNode node = Resolve(abs, user, err);
		if (!node)
			return false;

		if (node.m_bDir)
		{
			err = ERR_IS_DIR;
			return false;
		}

		if (!node.HasPermission(user, 0))
		{
			err = ERR_PERMISSION;
			return false;
		}

		content = node.m_sContent;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes a node. Non-empty directories need recursive.
	bool Remove(string abs, string user, bool recursive, out string err)
	{
		if (abs == "/")
		{
			err = ERR_PERMISSION;
			return false;
		}

		ROOT_AE3_FsNode node = Resolve(abs, user, err, false);
		if (!node)
			return false;

		if (!node.m_sMountId.IsEmpty())
		{
			err = "Device or resource busy";
			return false;
		}

		if (node.m_bDir && !recursive && node.m_mChildren && !node.m_mChildren.IsEmpty())
		{
			err = ERR_NOT_EMPTY;
			return false;
		}

		ROOT_AE3_FsNode dir = WritableParent(abs, user, err);
		if (!dir)
			return false;

		if (!MayReplace(dir, node, user))
		{
			err = ERR_PERMISSION;
			return false;
		}

		dir.m_mChildren.Remove(node.m_sName);
		Changed(abs);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! In a folder everyone may write, only root and the owners of the file or folder replace or delete it.
	protected bool MayReplace(ROOT_AE3_FsNode dir, ROOT_AE3_FsNode node, string user)
	{
		if (user == "root" || (dir.m_iPerms & 16) == 0)
			return true;

		return node.m_sOwner == user || dir.m_sOwner == user;
	}

	//------------------------------------------------------------------------------------------------
	//! Copies src to dst. Directory destination keeps the source name. Works across filesystems.
	bool Copy(string src, string dst, string user, out string err, ROOT_AE3_Filesystem dstFs = null, bool move = false)
	{
		if (!dstFs)
			dstFs = this;

		ROOT_AE3_FsNode node = Resolve(src, user, err, false);
		if (!node)
			return false;

		if (!node.HasPermission(user, 0) || (node.m_bDir && !node.HasPermission(user, 2)))
		{
			err = ERR_PERMISSION;
			return false;
		}

		ROOT_AE3_FsNode target = dstFs.Resolve(dst, user, err);
		if (target && target.m_bDir)
		{
			dst = dst + "/" + node.m_sName;
			if (dst.StartsWith("//"))
				dst = dst.Substring(1, dst.Length() - 1);
		}

		if (dstFs == this && (dst == src || dst.StartsWith(src + "/")))
		{
			err = "Cannot copy a directory into itself";
			return false;
		}

		ROOT_AE3_FsNode existing = dstFs.Resolve(dst, user, err, false);
		if (existing && existing.m_bDir)
		{
			err = ERR_EXISTS;
			return false;
		}

		ROOT_AE3_FsNode dir = dstFs.WritableParent(dst, user, err);
		if (!dir)
			return false;

		if (existing && (!existing.HasPermission(user, 1) || !MayReplace(dir, existing, user)))
		{
			err = ERR_PERMISSION;
			return false;
		}

		if (node.m_bDir && node.Holds(dir))
		{
			err = "Cannot copy a directory into itself";
			return false;
		}

		ROOT_AE3_Filesystem holder = dstFs.GetLastFs();
		bool sameFs = holder == GetLastFsOf(src, user);
		if (!(move && sameFs) && !holder.Fits(node, err))
			return false;

		string name = BaseName(dst);
		if (!ValidName(name))
		{
			err = ERR_BAD_PATH;
			return false;
		}

		if (move)
		{
			if (!node.m_sMountId.IsEmpty())
			{
				err = "Device or resource busy";
				return false;
			}

			ROOT_AE3_FsNode srcDir = WritableParent(src, user, err);
			if (!srcDir)
				return false;

			if (!MayReplace(srcDir, node, user))
			{
				err = ERR_PERMISSION;
				return false;
			}

			ROOT_AE3_FsNode keep = node;
			srcDir.m_mChildren.Remove(node.m_sName);
			keep.m_sName = name;
			dir.SetChild(keep);
			err = "";
			Changed(src);
			dstFs.Changed(dst);
			return true;
		}

		ROOT_AE3_FsNode c = node.CopyReadable(user);
		if (!c)
		{
			err = ERR_PERMISSION;
			return false;
		}

		c.m_sName = name;
		if (user != "root")
			c.SetOwnerRecursive(user);

		dir.SetChild(c);
		err = "";
		dstFs.Changed(dst);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected ROOT_AE3_Filesystem GetLastFsOf(string abs, string user)
	{
		string e;
		Resolve(abs, user, e, false);
		return GetLastFs();
	}

	//------------------------------------------------------------------------------------------------
	bool Move(string src, string dst, string user, out string err, ROOT_AE3_Filesystem dstFs = null)
	{
		if (dstFs && dstFs != this)
		{
			if (!Copy(src, dst, user, err, dstFs))
				return false;

			return Remove(src, user, true, err);
		}

		return Copy(src, dst, user, err, null, true);
	}

	//------------------------------------------------------------------------------------------------
	bool Chown(string abs, string newOwner, string user, out string err)
	{
		ROOT_AE3_FsNode node = Resolve(abs, user, err, false);
		if (!node)
			return false;

		if (user != "root")
		{
			err = ERR_PERMISSION;
			return false;
		}

		node.m_sOwner = newOwner;
		Changed(abs);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool Chmod(string abs, int perms, string user, out string err)
	{
		ROOT_AE3_FsNode node = Resolve(abs, user, err, false);
		if (!node)
			return false;

		if (user != "root" && node.m_sOwner != user)
		{
			err = ERR_PERMISSION;
			return false;
		}

		node.m_iPerms = perms;
		Changed(abs);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool Symlink(string target, string linkPath, string user, out string err)
	{
		if (Resolve(linkPath, user, err, false))
		{
			err = ERR_EXISTS;
			return false;
		}

		if (!ValidName(BaseName(linkPath)) || target.Length() > 256)
		{
			err = ERR_BAD_PATH;
			return false;
		}

		ROOT_AE3_FsNode dir = WritableParent(linkPath, user, err);
		if (!dir || !GetLastFs().Fits(null, err, target.Length(), 1))
			return false;

		ROOT_AE3_FsNode l = new ROOT_AE3_FsNode(BaseName(linkPath), false, user, 63);
		l.m_sLink = target;
		dir.SetChild(l);
		Changed(linkPath);
		err = "";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Paths below abs whose name matches the regex pattern (case-insensitive).
	void Find(string abs, string pattern, string user, notnull array<string> results, int limit = 500)
	{
		string err;
		ROOT_AE3_FsNode node = Resolve(abs, user, err);
		if (!node || !node.m_bDir)
			return;

		FindIn(node, abs, pattern, user, results, limit, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void FindIn(ROOT_AE3_FsNode dir, string path, string pattern, string user, array<string> results, int limit, int depth)
	{
		if (depth > 32 || !dir.HasPermission(user, 0) || !dir.HasPermission(user, 2))
			return;

		foreach (string name : dir.GetSortedNames())
		{
			if (results.Count() >= limit)
				return;

			ROOT_AE3_FsNode c = dir.GetChild(name);
			string p = path + "/" + name;
			if (path == "/")
				p = "/" + name;

			if (ROOT_AE3_Regex.LimitHit())
				return;

			if (ROOT_AE3_Regex.Search(pattern, name, true))
				results.Insert(p);

			if (c.m_bDir)
			{
				ROOT_AE3_FsNode inner = c;
				if (!c.m_sMountId.IsEmpty() && m_mMounts.Get(c.m_sMountId))
					inner = m_mMounts.Get(c.m_sMountId).m_Root;

				FindIn(inner, p, pattern, user, results, limit, depth + 1);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Moves abs into /.trash and remembers its original path.
	bool Trash(string abs, string user, out string err)
	{
		ROOT_AE3_FsNode node = Resolve(abs, user, err, false);
		if (!node)
			return false;

		if (GetLastFs() != this)
		{
			if (!Remove(abs, user, true, err))
				return false;

			err = "deleted";
			return true;
		}

		ROOT_AE3_FsNode parent = WritableParent(abs, user, err);
		if (!parent)
			return false;

		if (!MayReplace(parent, node, user))
		{
			err = ERR_PERMISSION;
			return false;
		}

		EnsureTrash();
		string name = BaseName(abs);
		string entry = name;
		int n = 1;
		while (Exists("/.trash/" + entry))
		{
			entry = name + "." + n.ToString();
			n++;
		}

		if (!Move(abs, "/.trash/" + entry, "root", err))
			return false;

		m_mTrashMeta.Set(entry, abs);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! /.trash exists and only root writes it.
	void EnsureTrash()
	{
		string e;
		ROOT_AE3_FsNode t = m_Root.GetChild(".trash");
		if (!t)
			CreateDir("/.trash", "root", "root", 7, e);
		else
			t.m_iPerms = 7;
	}

	//------------------------------------------------------------------------------------------------
	bool Restore(string entry, string user, out string err)
	{
		string orig = m_mTrashMeta.Get(entry);
		if (orig.IsEmpty())
			orig = "/tmp/" + entry;

		if (Exists(orig))
		{
			err = ERR_EXISTS;
			return false;
		}

		if (!WritableParent(orig, user, err))
			return false;

		if (!Move("/.trash/" + entry, orig, "root", err))
			return false;

		m_mTrashMeta.Remove(entry);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void EmptyTrash()
	{
		ROOT_AE3_FsNode t = m_Root.GetChild(".trash");
		if (t && t.m_mChildren)
			t.m_mChildren.Clear();

		Changed("/.trash");

		m_mTrashMeta.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Flat pre-order dump: one record per node, fields path, dir, owner, perms, size, link, mount, content.
	string Serialize()
	{
		array<string> records = {};
		SerializeNode(m_Root, "", records);
		array<string> trash = {};
		for (int i = 0, n = m_mTrashMeta.Count(); i < n; i++)
		{
			trash.Insert(m_mTrashMeta.GetKey(i));
			trash.Insert(m_mTrashMeta.GetElement(i));
		}

		records.Insert("#TRASH" + ROOT_AE3_Codec.Sep2() + ROOT_AE3_Codec.JoinSub(trash));
		return ROOT_AE3_Codec.Join(records);
	}

	//------------------------------------------------------------------------------------------------
	protected void SerializeNode(ROOT_AE3_FsNode node, string path, array<string> records)
	{
		array<string> f = {path, ROOT_AE3_Util.BoolStr(node.m_bDir), node.m_sOwner, node.m_iPerms.ToString(), node.m_iSize.ToString(), node.m_sLink, node.m_sMountId, node.m_sContent};
		records.Insert(ROOT_AE3_Codec.JoinSub(f));
		if (!node.m_mChildren)
			return;

		foreach (string name : node.GetSortedNames())
		{
			SerializeNode(node.GetChild(name), path + "/" + name, records);
		}
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_Filesystem Deserialize(string data)
	{
		ROOT_AE3_Filesystem fs = new ROOT_AE3_Filesystem();
		foreach (string rec : ROOT_AE3_Codec.Split(data))
		{
			array<string> f = ROOT_AE3_Codec.SplitSub(rec);
			if (f.IsEmpty())
				continue;

			if (f[0] == "#TRASH")
			{
				for (int i = 1; i + 1 < f.Count(); i += 2)
				{
					fs.m_mTrashMeta.Set(f[i], f[i + 1]);
				}

				continue;
			}

			if (f.Count() < 8)
				continue;

			ROOT_AE3_FsNode node;
			if (f[0].IsEmpty())
			{
				node = fs.m_Root;
			}
			else
			{
				string e;
				ROOT_AE3_FsNode parent = fs.Resolve(Parent(f[0]), "root", e, false);
				if (!parent || !parent.m_bDir)
					continue;

				node = new ROOT_AE3_FsNode(BaseName(f[0]), f[1] == "1", f[2], f[3].ToInt());
				parent.SetChild(node);
			}

			node.m_sOwner = f[2];
			node.m_iPerms = f[3].ToInt();
			node.m_iSize = f[4].ToInt();
			node.m_sLink = f[5];
			node.m_sMountId = f[6];
			node.m_sContent = f[7];
			for (int j = 8; j < f.Count(); j++)
			{
				node.m_sContent += ROOT_AE3_Codec.Sep2() + f[j];
			}
		}

		return fs;
	}
}
