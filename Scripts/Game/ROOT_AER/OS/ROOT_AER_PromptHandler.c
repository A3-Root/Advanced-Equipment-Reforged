//! Receives the next line typed at a terminal prompt (passwords, confirmations).
class ROOT_AER_PromptHandler
{
	//------------------------------------------------------------------------------------------------
	//! Return true when the shell may show its prompt again.
	bool OnInput(ROOT_AER_ShellContext ctx, string input)
	{
		return true;
	}
}
