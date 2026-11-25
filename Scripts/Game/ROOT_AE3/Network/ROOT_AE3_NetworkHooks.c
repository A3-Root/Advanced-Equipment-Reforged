//! Condition for network links, registered by other mods (ROOT_AE3_API.AddConnectCondition). Server.
class ROOT_AE3_ConnectCondition : Managed
{
	//------------------------------------------------------------------------------------------------
	//! False blocks the link (checked on connect and every wireless sweep); reason is shown to the player.
	bool Allow(ROOT_AE3_NetworkComponent client, ROOT_AE3_NetworkComponent router, out string reason)
	{
		return true;
	}
}

//! Wireless range modifier, registered by other mods (ROOT_AE3_API.AddRangeModifier). Server.
class ROOT_AE3_RangeModifier : Managed
{
	//------------------------------------------------------------------------------------------------
	//! Returns the range in m to use for router (jamming, terrain, weather).
	float Modify(ROOT_AE3_NetworkComponent router, float range)
	{
		return range;
	}
}
