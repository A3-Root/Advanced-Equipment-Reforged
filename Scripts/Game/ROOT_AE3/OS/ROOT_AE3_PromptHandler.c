//! Receives the next line typed at a terminal prompt (passwords, confirmations).
class ROOT_AE3_PromptHandler
{
	//------------------------------------------------------------------------------------------------
	//! Return true when the shell may show its prompt again.
	bool OnInput(ROOT_AE3_ShellContext ctx, string input)
	{
		return true;
	}
}
