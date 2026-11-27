//! Adds Advanced Equipment Reforged settings to the mission header.
modded class SCR_MissionHeader
{
	[Attribute(desc: "Advanced Equipment Reforged settings. Empty = defaults.")]
	ref ROOT_AER_Settings m_ROOT_AER_Settings;
}
