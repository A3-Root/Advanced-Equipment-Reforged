//! User account set in the World Editor.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class ROOT_AE3_UserEntry
{
	[Attribute("", desc: "User name")]
	string m_sName;

	[Attribute("", desc: "Password")]
	string m_sPassword;

	[Attribute("0", desc: "Listed in /etc/sudoers")]
	bool m_bSudoer;
}
