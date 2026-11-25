//! Server handler of a desktop request registered by another mod (ROOT_AE3_API.RegisterGuiCommand).
//! Reply with ROOT_AE3_GuiRouter.Reply / Msg using the given rid and cmd.
class ROOT_AE3_GuiCommand : Managed
{
	//------------------------------------------------------------------------------------------------
	//! fields: request fields after rid and cmd. user: logged-in desktop user.
	void Handle(ROOT_AE3_Session session, string rid, string cmd, array<string> fields, string user)
	{
	}
}
