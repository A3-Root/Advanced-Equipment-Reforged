//! Opens the computer menu for the local player. m_iMode: 0 terminal, 1 desktop.
class ROOT_AE3_UseComputerAction : ROOT_AE3_UserActionBase
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, desc: "Interface", enums: { ParamEnum("Terminal", "0"), ParamEnum("Desktop", "1") })]
	protected int m_iMode;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (ROOT_AE3_Util.IsDestroyed(GetOwner()))
			return false;

		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(GetOwner());
		if (!c || !IsPlaced(GetOwner()))
			return false;

		int iface = c.GetInterfaceMode();
		if (m_iMode == 0 && iface == 1)
			return false;

		if (m_iMode == 1 && iface == 0)
			return false;

		ROOT_AE3_LidComponent lid = ROOT_AE3_LidComponent.Of(GetOwner());
		if (lid && !lid.IsOpen())
			return false;

		int screen = c.GetScreen();
		return screen != ROOT_AE3_EScreen.OFF && screen != ROOT_AE3_EScreen.SHUTDOWN && screen != ROOT_AE3_EScreen.CRASHED;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(GetOwner());
		ROOT_AE3_LidComponent lid = ROOT_AE3_LidComponent.Of(GetOwner());
		if (lid && !lid.IsOpen())
		{
			SetCannotPerformReason("Lid closed");
			return false;
		}

		int screen = c.GetScreen();
		if (screen == ROOT_AE3_EScreen.OFF || screen == ROOT_AE3_EScreen.SHUTDOWN || screen == ROOT_AE3_EScreen.CRASHED)
		{
			SetCannotPerformReason("Not running");
			return false;
		}

		if (screen == ROOT_AE3_EScreen.BOOT)
		{
			SetCannotPerformReason("Starting");
			return false;
		}

		if (c.IsInUse())
		{
			SetCannotPerformReason("In use");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (pUserEntity != SCR_PlayerController.GetLocalControlledEntity())
			return;

		ROOT_AE3_Client.Get().Open(pOwnerEntity, m_iMode);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_iMode == 1)
			outName = "Use desktop";
		else
			outName = "Use terminal";

		return true;
	}
}
