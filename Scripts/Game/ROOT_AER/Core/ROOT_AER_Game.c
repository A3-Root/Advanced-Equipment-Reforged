//! Resets Advanced Equipment Reforged singletons when a game (world) ends, so the next mission starts clean.
modded class ArmaReforgerScripted
{
	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		ROOT_AER_Manager.SetEnding();
		ROOT_AER_Manager.Reset();
		ROOT_AER_PowerSystem.Reset();
		ROOT_AER_EwoSystem.Reset();
		ROOT_AER_NetworkSystem.Reset();
		ROOT_AER_GameTableSystem.Reset();
		ROOT_AER_Client.Reset();
		ROOT_AER_SettingsManager.Reset();
		ROOT_AER_SystemModuleComponent.Reset();
		ROOT_AER_Intel.Reset();
		ROOT_AER_Cctv.Reset();
		ROOT_AER_CableHand.Reset();
		ROOT_AER_Job.Reset();
		ROOT_AER_Net.Reset();
		ROOT_AER_ComputerComponent.Reset();
		ROOT_AER_CommandRegistry.Reset();
		ROOT_AER_API.Reset();
		super.OnGameEnd();
	}
}
