//! Extra condition for terminal / desktop access, registered by missions or mods (ROOT_AE3_API.AddAccessCondition). Server.
class ROOT_AE3_AccessCondition : Managed
{
	//------------------------------------------------------------------------------------------------
	//! mode 0 terminal, 1 desktop. False denies access after the access lists allowed it.
	bool Allow(int playerId, ROOT_AE3_ComputerComponent computer, int mode)
	{
		return true;
	}
}
