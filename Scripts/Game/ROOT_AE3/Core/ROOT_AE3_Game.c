//! Resets Advanced Equipment singletons when a game (world) ends, so the next mission starts clean.
modded class ArmaReforgerScripted
{
	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		ROOT_AE3_Manager.SetEnding();
		ROOT_AE3_Manager.Reset();
		ROOT_AE3_PowerSystem.Reset();
		ROOT_AE3_NetworkSystem.Reset();
		ROOT_AE3_GameTableSystem.Reset();
		ROOT_AE3_Client.Reset();
		ROOT_AE3_SettingsManager.Reset();
		ROOT_AE3_Intel.Reset();
		ROOT_AE3_Cctv.Reset();
		ROOT_AE3_Job.Reset();
		ROOT_AE3_Net.Reset();
		ROOT_AE3_ComputerComponent.Reset();
		ROOT_AE3_CommandRegistry.Reset();
		ROOT_AE3_API.Reset();
		super.OnGameEnd();
	}
}
