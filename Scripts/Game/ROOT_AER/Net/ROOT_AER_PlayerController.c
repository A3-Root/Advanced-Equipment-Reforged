//! Client <-> server transport of Advanced Equipment Reforged messages.
//! RpcAsk_ROOT_AER: owning client -> server. RpcDo_ROOT_AER: server -> owning client.
//! SP and listen host: the local player's messages are handled directly.
modded class SCR_PlayerController
{
	//------------------------------------------------------------------------------------------------
	//! Local client: sends a message to the server.
	void ROOT_AER_Ask(RplId deviceId, int op, string data)
	{
		if (Replication.IsServer())
		{
			ROOT_AER_Manager.Get().OnClientMessage(GetPlayerId(), deviceId, op, data);
			return;
		}

		Rpc(RpcAsk_ROOT_AER, deviceId, op, data);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ROOT_AER(RplId deviceId, int op, string data)
	{
		ROOT_AER_Manager.Get().OnClientMessage(GetPlayerId(), deviceId, op, data);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: sends a message to the client owning this controller.
	void ROOT_AER_Do(int op, string data)
	{
		if (GetGame().GetPlayerController() == this)
		{
			ROOT_AER_Client.Get().OnServerMessage(op, data);
			return;
		}

		Rpc(RpcDo_ROOT_AER, op, data);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_ROOT_AER(int op, string data)
	{
		ROOT_AER_Client.Get().OnServerMessage(op, data);
	}
}
