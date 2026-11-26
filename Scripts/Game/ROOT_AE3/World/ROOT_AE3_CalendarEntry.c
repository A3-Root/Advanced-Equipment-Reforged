//! Calendar event.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sTitle")]
class ROOT_AE3_CalendarEntry
{
	[Attribute("", desc: "Date YYYY-MM-DD; empty = mission date")]
	string m_sDate;

	[Attribute("", desc: "Time HH:MM")]
	string m_sTime;

	[Attribute("", desc: "Title")]
	string m_sTitle;

	[Attribute("", uiwidget: UIWidgets.EditBoxMultiline, desc: "Description")]
	string m_sText;

	[Attribute("", desc: "Location")]
	string m_sLocation;
}
