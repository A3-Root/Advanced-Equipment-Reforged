//! Message helpers with chunking of long payloads (CHUNK parts, then the real message).
class ROOT_AE3_Net
{
	static const int CHUNK_SIZE = 6000;
	static const int BUDGET = 65536;	//!< Characters per player and second; larger bursts are dropped
	protected static ref array<int> s_aDropped = {};
	protected static ref array<int> s_aNotified = {};	//!< Players told about dropped output in this window	//!< Players whose terminal output was dropped
	protected static ref map<int, int> s_mSent = new map<int, int>();
	protected static float s_fWindow;

	//------------------------------------------------------------------------------------------------
	//! Game end.
	static void Reset()
	{
		s_aDropped.Clear();
		s_mSent.Clear();
		s_fWindow = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: true once after terminal output to the player was dropped.
	static bool TakeDropped(int playerId)
	{
		if (!s_aDropped.Contains(playerId))
			return false;

		s_aDropped.RemoveItem(playerId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Server: message to one player.
	static void SendToPlayer(int playerId, ROOT_AE3_EMsg op, string data)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!pc)
			return;

		float now = GetGame().GetWorld().GetWorldTime() / 1000.0;
		if (now - s_fWindow >= 1 || now < s_fWindow)
		{
			s_fWindow = now;
			s_mSent.Clear();
			s_aNotified.Clear();
		}

		int sent = s_mSent.Get(playerId) + data.Length();
		s_mSent.Set(playerId, sent);
		bool essential = op == ROOT_AE3_EMsg.SESSION || op == ROOT_AE3_EMsg.ENDED || op == ROOT_AE3_EMsg.MODE || (op == ROOT_AE3_EMsg.GUI_REPLY && data.Length() <= 1024);
		if (sent - data.Length() > BUDGET && !essential)
		{
			if (!s_aNotified.Contains(playerId))
			{
				s_aNotified.Insert(playerId);
				array<string> f = {"Advanced Equipment", "Output limit reached; part of the output was dropped."};
				pc.ROOT_AE3_Do(ROOT_AE3_EMsg.NOTIFY, ROOT_AE3_Codec.Join(f));
			}

			if ((op == ROOT_AE3_EMsg.TERM_OUT || op == ROOT_AE3_EMsg.TERM_CLEAR) && !s_aDropped.Contains(playerId))
				s_aDropped.Insert(playerId);

			if (op == ROOT_AE3_EMsg.GUI_REPLY)
			{
				array<string> head = ROOT_AE3_Codec.Split(data.Substring(0, Math.Min(data.Length(), 256)));
				if (head.Count() >= 3)
				{
					array<string> reply = {head[0], "0", head[2], "Output limit reached, try again."};
					pc.ROOT_AE3_Do(ROOT_AE3_EMsg.GUI_REPLY, ROOT_AE3_Codec.Join(reply));
				}
			}

			return;
		}

		while (data.Length() > CHUNK_SIZE)
		{
			pc.ROOT_AE3_Do(ROOT_AE3_EMsg.CHUNK, data.Substring(0, CHUNK_SIZE));
			data = data.Substring(CHUNK_SIZE, data.Length() - CHUNK_SIZE);
		}

		pc.ROOT_AE3_Do(op, data);
	}

	//------------------------------------------------------------------------------------------------
	//! Client: message to the server about a device.
	static void SendToServer(RplId deviceId, ROOT_AE3_EMsg op, string data)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return;

		while (data.Length() > CHUNK_SIZE)
		{
			pc.ROOT_AE3_Ask(deviceId, ROOT_AE3_EMsg.CHUNK, data.Substring(0, CHUNK_SIZE));
			data = data.Substring(CHUNK_SIZE, data.Length() - CHUNK_SIZE);
		}

		pc.ROOT_AE3_Ask(deviceId, op, data);
	}
}
