//! Mail stored in /var/mail of a computer.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sSubject")]
class ROOT_AER_EmailEntry
{
	[Attribute("hq@rootnet.home", desc: "Sender")]
	string m_sFrom;

	[Attribute("", desc: "Recipient; empty = first address of the computer")]
	string m_sTo;

	[Attribute("", desc: "Subject")]
	string m_sSubject;

	[Attribute("", uiwidget: UIWidgets.EditBoxMultiline, desc: "Text")]
	string m_sBody;

	[Attribute("", desc: "Date YYYY-MM-DD HH:MM:SS; empty = mission time")]
	string m_sDate;
}
