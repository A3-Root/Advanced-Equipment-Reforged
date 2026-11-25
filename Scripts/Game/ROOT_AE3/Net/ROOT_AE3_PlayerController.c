//! Client <-> server transport of Advanced Equipment messages.
//! RpcAsk_ROOT_AE3: owning client -> server. RpcDo_ROOT_AE3: server -> owning client.
//! SP and listen host: the local player's messages are handled directly.
modded class SCR_PlayerController
{
	//------------------------------------------------------------------------------------------------
	//! Local client: sends a message to the server.
	void ROOT_AE3_Ask(RplId deviceId, int op, string data)
	{
		if (Replication.IsServer())
		{
			ROOT_AE3_Manager.Get().OnClientMessage(GetPlayerId(), deviceId, op, data);
			return;
		}

		Rpc(RpcAsk_ROOT_AE3, deviceId, op, data);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ROOT_AE3(RplId deviceId, int op, string data)
	{
		ROOT_AE3_Manager.Get().OnClientMessage(GetPlayerId(), deviceId, op, data);
	}

	//------------------------------------------------------------------------------------------------
	//! Server: sends a message to the client owning this controller.
	void ROOT_AE3_Do(int op, string data)
	{
		if (GetGame().GetPlayerController() == this)
		{
			ROOT_AE3_Client.Get().OnServerMessage(op, data);
			return;
		}

		Rpc(RpcDo_ROOT_AE3, op, data);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_ROOT_AE3(int op, string data)
	{
		ROOT_AE3_Client.Get().OnServerMessage(op, data);
	}
}
