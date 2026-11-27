//! In-game web page.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sUrl")]
class ROOT_AER_WebpageEntry
{
	[Attribute("intel.local", desc: "Address")]
	string m_sUrl;

	[Attribute("", desc: "Title")]
	string m_sTitle;

	[Attribute("", uiwidget: UIWidgets.EditBoxMultiline, desc: "Markdown content: # headings, - lists, **bold**, [text](address)")]
	string m_sContent;

	[Attribute("0", desc: "Visible on every computer instead of this one only")]
	bool m_bGlobal;
}
