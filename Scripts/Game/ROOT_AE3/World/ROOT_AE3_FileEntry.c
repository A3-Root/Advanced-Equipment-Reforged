//! File or directory placed on a computer by the World Editor, Game Master or API.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sPath")]
class ROOT_AE3_FileEntry
{
	[Attribute("/home/user/notes.txt", desc: "Absolute path")]
	string m_sPath;

	[Attribute("0", desc: "Directory instead of file")]
	bool m_bDirectory;

	[Attribute("", uiwidget: UIWidgets.EditBoxMultiline, desc: "Content")]
	string m_sContent;

	[Attribute("root", desc: "Owner")]
	string m_sOwner;

	[Attribute("rw-r--", desc: "Permissions: owner rwx then everyone rwx, '-' = not set")]
	string m_sPermissions;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Encryption of the content (ROOT_AE3_Encryption)", enums: { ParamEnum("None", "0"), ParamEnum("Caesar", "1"), ParamEnum("Columnar", "2"), ParamEnum("Vigenere", "3"), ParamEnum("ROT", "4"), ParamEnum("Affine", "5"), ParamEnum("Rail fence", "6"), ParamEnum("Morse", "7"), ParamEnum("Spelling alphabet", "8"), ParamEnum("Bacon", "9"), ParamEnum("Substitution", "10"), ParamEnum("Base32", "11"), ParamEnum("Base64", "12"), ParamEnum("Ascii85", "13"), ParamEnum("Unicode", "14"), ParamEnum("Integer", "15") })]
	int m_iEncryption;

	[Attribute("", desc: "Encryption key: Caesar shift, Columnar / Vigenere keyword, ROT variant, Affine a,b, rail count, Bacon 'extended', 26-letter alphabet, Integer radix; empty = default")]
	string m_sKey;

	[Attribute("", desc: "Password protection (unlock FILE PASSWORD); empty = none")]
	string m_sPassword;

	[Attribute("-1", desc: "Virtual size in bytes, -1 = content length")]
	int m_iSize;

	//------------------------------------------------------------------------------------------------
	//! Content after encryption and locking.
	string BuildContent()
	{
		string c = m_sContent;
		if (m_iEncryption > 0)
		{
			string key = m_sKey;
			if (key.IsEmpty())
				key = ROOT_AE3_Encryption.DefaultKey(m_iEncryption);

			c = ROOT_AE3_Encryption.Apply(m_iEncryption, c, key);
		}

		if (!m_sPassword.IsEmpty())
			c = ROOT_AE3_Intel.Lock(c, m_sPassword);

		return c;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: creates the entry on a filesystem.
	bool Apply(ROOT_AE3_Filesystem fs, out string err)
	{
		m_sOwner = ROOT_AE3_Codec.Clean(m_sOwner);
		if (m_sOwner.IsEmpty())
			m_sOwner = "root";

		string path = ROOT_AE3_Filesystem.Normalize("/", m_sPath, "root");
		string owner = m_sOwner;
		if (owner.IsEmpty())
			owner = "root";

		fs.CreateDir(ROOT_AE3_Filesystem.Parent(path), "root", "root", ROOT_AE3_FsNode.PERM_PUBLIC_DIR, err, true);
		if (m_bDirectory)
			return fs.CreateDir(path, "root", owner, ROOT_AE3_FsNode.ParsePerms(m_sPermissions, ROOT_AE3_FsNode.PERM_PUBLIC_DIR), err, true);

		if (!fs.WriteFile(path, BuildContent(), "root", false, err, ROOT_AE3_FsNode.ParsePerms(m_sPermissions, ROOT_AE3_FsNode.PERM_PUBLIC_FILE), owner))
			return false;

		ROOT_AE3_FsNode node = fs.Resolve(path, "root", err);
		if (node)
		{
			node.m_sOwner = owner;
			node.m_iPerms = ROOT_AE3_FsNode.ParsePerms(m_sPermissions, ROOT_AE3_FsNode.PERM_PUBLIC_FILE);
			node.m_iSize = m_iSize;
		}

		return true;
	}
}
