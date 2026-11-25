//! Game Master requests: action key, target devices and dialog values (server checks the rights).
modded class SCR_PlayerController
{
	//------------------------------------------------------------------------------------------------
	//! Local Game Master: sends an action for devices.
	void ROOT_AE3_GmRequest(string key, notnull array<RplId> targets, string values)
	{
		if (Replication.IsServer())
		{
			m_sROOT_AE3_GmBuffer = "";
			RpcAsk_ROOT_AE3_Gm(key, targets, values);
			return;
		}

		while (values.Length() > ROOT_AE3_Net.CHUNK_SIZE)
		{
			Rpc(RpcAsk_ROOT_AE3_GmChunk, values.Substring(0, ROOT_AE3_Net.CHUNK_SIZE));
			values = values.Substring(ROOT_AE3_Net.CHUNK_SIZE, values.Length() - ROOT_AE3_Net.CHUNK_SIZE);
		}

		Rpc(RpcAsk_ROOT_AE3_Gm, key, targets, values);
	}

	protected string m_sROOT_AE3_GmBuffer;
	protected bool m_bROOT_AE3_GmDropped;

	//------------------------------------------------------------------------------------------------
	//! Leading parts of long dialog values.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ROOT_AE3_GmChunk(string part)
	{
		if (m_bROOT_AE3_GmDropped)
			return;

		if (!ROOT_AE3_GmActions.IsGameMaster(GetPlayerId()) || m_sROOT_AE3_GmBuffer.Length() + part.Length() > 65536)
		{
			m_sROOT_AE3_GmBuffer = "";
			m_bROOT_AE3_GmDropped = true;
			return;
		}

		m_sROOT_AE3_GmBuffer += part;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ROOT_AE3_Gm(string key, array<RplId> targets, string values)
	{
		values = m_sROOT_AE3_GmBuffer + values;
		m_sROOT_AE3_GmBuffer = "";
		if (m_bROOT_AE3_GmDropped)
		{
			m_bROOT_AE3_GmDropped = false;
			return;
		}
		if (!ROOT_AE3_GmActions.IsGameMaster(GetPlayerId()) || !targets || targets.Count() > 64 || values.Length() > 65536 || key.Length() > 32)
			return;

		array<IEntity> ents = {};
		foreach (RplId id : targets)
		{
			ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.FromRplId(id);
			if (d)
				ents.Insert(d.GetOwner());
		}

		ROOT_AE3_GmActions.Execute(GetPlayerId(), key, ents, ROOT_AE3_Codec.Split(values));
	}
}
