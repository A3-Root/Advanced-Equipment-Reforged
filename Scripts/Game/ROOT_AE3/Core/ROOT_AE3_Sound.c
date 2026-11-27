//! Device sounds. Start, shut down, standby and flash drive: a chime (Sounds/ROOT_AE3/*.wav) for the player using the
//! device or standing within CHIME_RANGE, a switch sound at the device for the other players in range. Error, mail,
//! message, notification and click: the chime for the player using the device only.
class ROOT_AE3_Sound
{
	static const int START = 0;
	static const int STOP = 1;
	static const int STANDBY = 2;
	static const int ERROR = 3;
	static const int USB = 4;
	static const int MAIL = 5;
	static const int CLICK = 6;
	static const int MESSAGE = 7;
	static const int NOTIFY = 8;
	static const int USB_OUT = 9;
	static const float RANGE = 12;
	static const float CHIME_RANGE = 4;

	static const ResourceName SWITCH_ACP = "{288E6E06D5C624E7}Sounds/Props/Civilian/LightTable_01/Props_LightTable_01.acp";

	//------------------------------------------------------------------------------------------------
	//! Authority.
	static void Play(IEntity device, int soundId)
	{
		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		if (!d)
			return;

		if (!IsWorldSound(soundId))
		{
			ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Of(device);
			if (c && c.GetOperator() > 0)
				ROOT_AE3_Net.SendToPlayer(c.GetOperator(), ROOT_AE3_EMsg.SOUND, soundId.ToString());

			return;
		}

		d.BroadcastSound(soundId);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsWorldSound(int soundId)
	{
		return soundId == START || soundId == STOP || soundId == STANDBY || soundId == USB || soundId == USB_OUT;
	}

	//------------------------------------------------------------------------------------------------
	//! Client: sound of a device event.
	static void PlayAt(IEntity device, int soundId)
	{
		if (!IsWorldSound(soundId))
			return;

		string ev = "SOUND_TURN_OFF";
		if (soundId == START || soundId == USB)
			ev = "SOUND_TURN_ON";

		ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.Of(device);
		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		bool mine = d && ROOT_AE3_Client.Get().m_DeviceId == d.GetRplId();
		if (mine || (me && vector.Distance(me.GetOrigin(), ROOT_AE3_Util.WorldPos(device)) <= CHIME_RANGE))
		{
			PlayLocal(soundId);
			return;
		}

		vector mat[4];
		device.GetWorldTransform(mat);
		AudioSystem.PlayEvent(SWITCH_ACP, ev, mat);
	}

	//------------------------------------------------------------------------------------------------
	//! Client: the chime of an event.
	static void PlayLocal(int soundId)
	{
		ResourceName res;
		switch (soundId)
		{
			case START: res = "{898DA97E62E5F483}Sounds/ROOT_AE3/AE3_Startup.wav"; break;
			case STOP: res = "{1D97E9B025518A2A}Sounds/ROOT_AE3/AE3_Shutdown.wav"; break;
			case STANDBY: res = "{55FC688AEFF069A6}Sounds/ROOT_AE3/AE3_Standby.wav"; break;
			case ERROR: res = "{1A246A0665F9B54F}Sounds/ROOT_AE3/AE3_Error.wav"; break;
			case USB: res = "{2E6670F347150214}Sounds/ROOT_AE3/AE3_UsbIn.wav"; break;
			case USB_OUT: res = "{873B89C15A611338}Sounds/ROOT_AE3/AE3_UsbOut.wav"; break;
			case MAIL: res = "{E43DC78D75AB021C}Sounds/ROOT_AE3/AE3_Mail.wav"; break;
			case MESSAGE: res = "{AFAA5854A4BA7364}Sounds/ROOT_AE3/AE3_Message.wav"; break;
			case NOTIFY: res = "{AC225EEC66F18630}Sounds/ROOT_AE3/AE3_Notify.wav"; break;
			default: res = "{2C2C429F169253EB}Sounds/ROOT_AE3/AE3_Click.wav";
		}

		AudioSystem.PlaySound(res);
	}
}
