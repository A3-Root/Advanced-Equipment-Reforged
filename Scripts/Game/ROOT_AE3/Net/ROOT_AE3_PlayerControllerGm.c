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

	//------------------------------------------------------------------------------------------------
	//! Local Game Master: asks the server for the form fields of an action (ROOT_AE3_GmDialog.OnFields).
	void ROOT_AE3_GmQuery(string key, notnull array<RplId> targets)
	{
		if (Replication.IsServer())
		{
			RpcAsk_ROOT_AE3_GmQuery(key, targets);
			return;
		}

		Rpc(RpcAsk_ROOT_AE3_GmQuery, key, targets);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ROOT_AE3_GmQuery(string key, array<RplId> targets)
	{
		if (!ROOT_AE3_GmActions.IsGameMaster(GetPlayerId()) || !targets || targets.Count() > 64 || key.Length() > 32)
			return;

		array<IEntity> ents = {};
		foreach (RplId id : targets)
		{
			ROOT_AE3_DeviceComponent d = ROOT_AE3_DeviceComponent.FromRplId(id);
			if (d)
				ents.Insert(d.GetOwner());
		}

		string title = ROOT_AE3_GmActions.Label(key);
		array<ref ROOT_AE3_GmField> fields = {};
		ROOT_AE3_GmActions.Fields(key, ents, title, fields);
		string packed = ROOT_AE3_GmField.Pack(fields);
		if (packed.Length() > 60000)
			packed = "";

		if (fields.IsEmpty())
			ROOT_AE3_GmActions.Notify(GetPlayerId(), ROOT_AE3_GmActions.Label(key) + ": no matching device selected.");

		if (GetGame().GetPlayerController() == this)
			RpcDo_ROOT_AE3_GmFields(key, title, packed);
		else
			Rpc(RpcDo_ROOT_AE3_GmFields, key, title, packed);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_ROOT_AE3_GmFields(string key, string title, string packed)
	{
		ROOT_AE3_GmDialog.OnFields(key, title, packed);
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
