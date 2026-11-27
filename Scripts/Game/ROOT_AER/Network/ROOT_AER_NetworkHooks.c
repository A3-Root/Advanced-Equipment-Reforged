//! Condition for network links, registered by other mods (ROOT_AER_API.AddConnectCondition). Server.
class ROOT_AER_ConnectCondition : Managed
{
	//------------------------------------------------------------------------------------------------
	//! False blocks the link (checked on connect and every wireless sweep); reason is shown to the player.
	bool Allow(ROOT_AER_NetworkComponent client, ROOT_AER_NetworkComponent router, out string reason)
	{
		return true;
	}
}

//! Wireless range modifier, registered by other mods (ROOT_AER_API.AddRangeModifier). Server.
class ROOT_AER_RangeModifier : Managed
{
	//------------------------------------------------------------------------------------------------
	//! Returns the range in m to use for router (jamming, terrain, weather).
	float Modify(ROOT_AER_NetworkComponent router, float range)
	{
		return range;
	}
}
