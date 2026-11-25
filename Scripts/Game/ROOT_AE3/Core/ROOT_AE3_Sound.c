//! Device sounds for players near the device: switch sounds at the device position (vanilla light table
//! events), error / mail / click as UI sounds for the player using the device.
class ROOT_AE3_Sound
{
	static const int START = 0;
	static const int STOP = 1;
	static const int STANDBY = 2;
	static const int ERROR = 3;
	static const int USB = 4;
	static const int MAIL = 5;
	static const int CLICK = 6;
	static const float RANGE = 12;

	//------------------------------------------------------------------------------------------------
	//! Authority.
	static void Play(IEntity device, int soundId)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		if (!d)
			return;

		if (soundId == ERROR || soundId == MAIL || soundId == CLICK)
		{
			ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(device);
			if (c && c.GetOperator() > 0)
				ROOT_AE3_Net.SendToPlayer(c.GetOperator(), ROOT_AE3_EMsg.SOUND, soundId.ToString());

			return;
		}

		d.BroadcastSound(soundId);
	}

	static const ResourceName SWITCH_ACP = "{288E6E06D5C624E7}Sounds/Props/Civilian/LightTable_01/Props_LightTable_01.acp";

	//------------------------------------------------------------------------------------------------
	//! Client: sound of a device event.
	static void PlayAt(IEntity device, int soundId)
	{
		if (soundId == START || soundId == STOP || soundId == STANDBY || soundId == USB)
		{
			string ev = "SOUND_TURN_OFF";
			if (soundId == START || soundId == USB)
				ev = "SOUND_TURN_ON";

			vector mat[4];
			device.GetWorldTransform(mat);
			AudioSystem.PlayEvent(SWITCH_ACP, ev, mat);
			return;
		}

		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		if (d && ROOT_AE3_Client.Get().m_DeviceId == d.GetRplId())
			PlayLocal(soundId);
	}

	//------------------------------------------------------------------------------------------------
	//! Client.
	static void PlayLocal(int soundId)
	{
		switch (soundId)
		{
			case START: SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ITEM_CONFIRMED); break;
			case STOP: SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ITEM_CANCELLED); break;
			case STANDBY: SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ITEM_SELECTED); break;
			case ERROR: SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ERROR); break;
			case USB: SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_PICKUP_CLICK); break;
			case MAIL: SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.HINT); break;
			default: SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.CLICK);
		}
	}
}
