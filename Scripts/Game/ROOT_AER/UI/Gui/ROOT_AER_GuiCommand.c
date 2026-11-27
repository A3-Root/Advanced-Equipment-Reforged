//! Server handler of a desktop request registered by another mod (ROOT_AER_API.RegisterGuiCommand).
//! Reply with ROOT_AER_GuiRouter.Reply / Msg using the given rid and cmd.
class ROOT_AER_GuiCommand : Managed
{
	//------------------------------------------------------------------------------------------------
	//! fields: request fields after rid and cmd. user: logged-in desktop user.
	void Handle(ROOT_AER_Session session, string rid, string cmd, array<string> fields, string user)
	{
	}
}
