//! Extra condition for terminal / desktop access, registered by missions or mods (ROOT_AER_API.AddAccessCondition). Server.
class ROOT_AER_AccessCondition : Managed
{
	//------------------------------------------------------------------------------------------------
	//! mode 0 terminal, 1 desktop. False denies access after the access lists allowed it.
	bool Allow(int playerId, ROOT_AER_ComputerComponent computer, int mode)
	{
		return true;
	}
}
