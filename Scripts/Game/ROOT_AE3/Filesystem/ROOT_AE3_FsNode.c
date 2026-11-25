//! One file or directory of a virtual filesystem.
//! Permission bits: 1 owner read, 2 owner write, 4 owner execute, 8 everyone read, 16 everyone write, 32 everyone execute.
class ROOT_AE3_FsNode : Managed
{
	static const int P_OR = 1;
	static const int P_OW = 2;
	static const int P_OX = 4;
	static const int P_ER = 8;
	static const int P_EW = 16;
	static const int P_EX = 32;
	static const int PERM_FILE_DEFAULT = 3;		// rw- ---
	static const int PERM_DIR_DEFAULT = 7;		// rwx ---
	static const int PERM_PUBLIC_DIR = 47;		// rwx r-x
	static const int PERM_PUBLIC_FILE = 11;		// rw- r--

	string m_sName;
	bool m_bDir;
	string m_sContent;
	string m_sOwner = "root";
	int m_iPerms = PERM_FILE_DEFAULT;
	int m_iSize = -1;			//!< Virtual size in bytes, -1 = content length.
	string m_sLink;				//!< Symlink target path; empty for regular nodes.
	string m_sMountId;			//!< Mount point id (for example "usb0"), resolved by the owning filesystem.
	ref map<string, ref ROOT_AE3_FsNode> m_mChildren;

	//------------------------------------------------------------------------------------------------
	void ROOT_AE3_FsNode(string name, bool dir, string owner = "root", int perms = -1)
	{
		m_sName = name;
		m_bDir = dir;
		m_sOwner = owner;
		if (perms >= 0)
			m_iPerms = perms;
		else if (dir)
			m_iPerms = PERM_DIR_DEFAULT;

		if (dir)
			m_mChildren = new map<string, ref ROOT_AE3_FsNode>();
	}

	//------------------------------------------------------------------------------------------------
	bool IsLink()
	{
		return !m_sLink.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_FsNode GetChild(string name)
	{
		if (!m_mChildren)
			return null;

		return m_mChildren.Get(name);
	}

	//------------------------------------------------------------------------------------------------
	void SetChild(notnull ROOT_AE3_FsNode node)
	{
		if (!m_mChildren)
			m_mChildren = new map<string, ref ROOT_AE3_FsNode>();

		m_mChildren.Set(node.m_sName, node);
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetSortedNames()
	{
		array<string> names = {};
		if (!m_mChildren)
			return names;

		for (int i = 0, n = m_mChildren.Count(); i < n; i++)
		{
			names.Insert(m_mChildren.GetKey(i));
		}

		names.Sort();
		return names;
	}

	//------------------------------------------------------------------------------------------------
	//! Virtual size; directories sum their children.
	//! Number of nodes, recursive.
	int CountNodes()
	{
		int total = 1;
		if (m_bDir && m_mChildren)
		{
			for (int i = 0, n = m_mChildren.Count(); i < n; i++)
			{
				total += m_mChildren.GetElement(i).CountNodes();
			}
		}

		return total;
	}

	//------------------------------------------------------------------------------------------------
	//! Characters held in memory (names and contents), recursive.
	int GetStoredChars()
	{
		int total = m_sName.Length() + m_sContent.Length() + m_sLink.Length();
		if (m_bDir && m_mChildren)
		{
			for (int i = 0, n = m_mChildren.Count(); i < n; i++)
			{
				total += m_mChildren.GetElement(i).GetStoredChars();
			}
		}

		return total;
	}

	//------------------------------------------------------------------------------------------------
	//! Copy holding only what user may read (folders also need traverse); null when the node itself is unreadable.
	ROOT_AE3_FsNode CopyReadable(string user)
	{
		if (!HasPermission(user, 0) || (m_bDir && !HasPermission(user, 2)))
			return null;

		if (!m_bDir)
			return Copy();

		ROOT_AE3_FsNode c = new ROOT_AE3_FsNode(m_sName, true, m_sOwner, m_iPerms);
		if (m_mChildren)
		{
			for (int i = 0, n = m_mChildren.Count(); i < n; i++)
			{
				ROOT_AE3_FsNode child = m_mChildren.GetElement(i).CopyReadable(user);
				if (child)
					c.SetChild(child);
			}
		}

		return c;
	}

	//------------------------------------------------------------------------------------------------
	//! n is this node or below it.
	bool Holds(ROOT_AE3_FsNode n)
	{
		if (n == this)
			return true;

		if (!m_bDir || !m_mChildren)
			return false;

		for (int i = 0, c = m_mChildren.Count(); i < c; i++)
		{
			if (m_mChildren.GetElement(i).Holds(n))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetOwnerRecursive(string owner)
	{
		m_sOwner = owner;
		if (m_bDir && m_mChildren)
		{
			for (int i = 0, n = m_mChildren.Count(); i < n; i++)
			{
				m_mChildren.GetElement(i).SetOwnerRecursive(owner);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetSize()
	{
		if (m_bDir)
		{
			int total;
			if (m_mChildren)
			{
				for (int i = 0, n = m_mChildren.Count(); i < n; i++)
				{
					total += m_mChildren.GetElement(i).GetSize();
				}
			}

			return total;
		}

		if (m_iSize >= 0)
			return m_iSize;

		return m_sContent.Length();
	}

	//------------------------------------------------------------------------------------------------
	//! True when user may access in mode: 0 read, 1 write, 2 execute.
	bool HasPermission(string user, int mode)
	{
		if (user == "root" || m_sOwner.IsEmpty())
			return true;

		int bit = 1 << mode;
		if (m_sOwner == user)
			return (m_iPerms & bit) != 0;

		return (m_iPerms & (bit << 3)) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! "drwxr-x---" style string.
	string PermString()
	{
		string s = "-";
		if (m_bDir)
			s = "d";
		else if (IsLink())
			s = "l";

		array<string> letters = {"r", "w", "x", "r", "w", "x"};
		for (int i = 0; i < 6; i++)
		{
			if (m_iPerms & (1 << i))
				s += letters[i];
			else
				s += "-";
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! Deep copy.
	ROOT_AE3_FsNode Copy()
	{
		ROOT_AE3_FsNode c = new ROOT_AE3_FsNode(m_sName, m_bDir, m_sOwner, m_iPerms);
		c.m_sContent = m_sContent;
		c.m_iSize = m_iSize;
		c.m_sLink = m_sLink;
		if (m_mChildren)
		{
			for (int i = 0, n = m_mChildren.Count(); i < n; i++)
			{
				c.SetChild(m_mChildren.GetElement(i).Copy());
			}
		}

		return c;
	}

	//------------------------------------------------------------------------------------------------
	//! Parses "rwxr-x" (6 chars) or octal-like "75" (owner digit, everyone digit).
	static int ParsePerms(string s, int fallback)
	{
		if (s.Length() == 6)
		{
			int p;
			for (int i = 0; i < 6; i++)
			{
				if (s.Get(i) != "-")
					p |= 1 << i;
			}

			return p;
		}

		if (s.Length() == 2 && ROOT_AE3_Util.IsInt(s))
		{
			int o = s.Substring(0, 1).ToInt();
			int e = s.Substring(1, 1).ToInt();
			return OctalDigit(o) | (OctalDigit(e) << 3);
		}

		return fallback;
	}

	//------------------------------------------------------------------------------------------------
	//! Unix digit (r=4 w=2 x=1) to bit order r=1 w=2 x=4.
	protected static int OctalDigit(int d)
	{
		int r;
		if (d & 4)
			r |= 1;

		if (d & 2)
			r |= 2;

		if (d & 1)
			r |= 4;

		return r;
	}
}
