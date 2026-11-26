//! Starts the retro Snake game.
class ROOT_AE3_AppSnake : ROOT_AE3_App
{
	//------------------------------------------------------------------------------------------------
	override string GetTitle()
	{
		return "Snake";
	}

	//------------------------------------------------------------------------------------------------
	override string GetIcon()
	{
		return "snake";
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSize()
	{
		return Vector(300, 120, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void Build()
	{
		Label(10, 10, Width() - 20, 22, "Starting Snake...");
		Btn(10, 40, 120, 28, "Big blocks", 1);
		ROOT_AE3_Client.Get().m_Menu.StartSnake(false);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUiClick(Widget w, int id, int button)
	{
		if (id == 1)
		{
			ROOT_AE3_Client.Get().m_Menu.StartSnake(true);
			return true;
		}

		return false;
	}
}
