//! Persistence of cable / wireless links and network settings of a device. Server only.
//! Links are restored once the linked entity is loaded (PersistenceSystem.WhenAvailable).
class ROOT_AE3_ConnectionSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return ROOT_AE3_ConnectionComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull SaveContext context)
	{
		UUID provider = UUID.NULL_UUID;
		UUID router = UUID.NULL_UUID;
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(owner);
		if (p && p.GetProvider())
			provider = GetSystem().GetId(p.GetProvider().GetOwner());

		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(owner);
		string config;
		bool wired;
		float cable;
		string cableItem;
		if (n)
		{
			if (n.GetParent())
			{
				IEntity parent = n.GetParent().GetOwner();
				router = GetSystem().GetId(parent);
				ROOT_AE3_ConnectionComponent link = ROOT_AE3_ConnectionComponent.Cast(component);
				wired = n.IsRouter() || link.IsWiredTo(parent);
				cable = link.GetCableLength(parent);
				cableItem = link.GetCableItem(parent);
			}

			config = n.PackConfig();
		}

		context.WriteValue("version", 2);
		context.WriteValue("provider", provider);
		context.WriteValue("router", router);
		context.WriteValue("network", config);
		context.WriteValue("wired", wired);
		context.WriteValue("cable", cable);
		context.WriteValue("cableItem", cableItem);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull LoadContext context)
	{
		int version;
		UUID provider, router;
		string config;
		bool wired = true;
		float cable = -1;
		string cableItem;
		context.ReadValue("version", version);
		context.ReadValue("provider", provider);
		context.ReadValue("router", router);
		context.ReadValue("network", config);
		context.ReadValue("wired", wired);
		if (version >= 2)
		{
			context.ReadValue("cable", cable);
			context.ReadValue("cableItem", cableItem);
		}

		ROOT_AE3_ConnectionComponent.Cast(component).MarkRestored();
		ROOT_AE3_NetworkComponent n = ROOT_AE3_NetworkComponent.Of(owner);
		if (n && !config.IsEmpty())
			n.UnpackConfig(config);

		Tuple1<IEntity> ctx = new Tuple1<IEntity>(owner);
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(owner);
		if (provider.IsNull() && p && p.GetProvider())
			p.Disconnect();

		if (router.IsNull() && n && !n.IsRouter() && n.GetParent())
			ROOT_AE3_NetworkSystem.Get().Disconnect(n);

		if (!provider.IsNull())
		{
			PersistenceWhenAvailableTask task = new PersistenceWhenAvailableTask(OnProvider, ctx);
			GetSystem().WhenAvailable(provider, task, 30);
		}

		if (!router.IsNull())
		{
			Tuple4<IEntity, bool, float, string> rctx = new Tuple4<IEntity, bool, float, string>(owner, wired, cable, cableItem);
			PersistenceWhenAvailableTask task2 = new PersistenceWhenAvailableTask(OnRouter, rctx);
			GetSystem().WhenAvailable(router, task2, 30);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnProvider(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		IEntity source = IEntity.Cast(instance);
		Tuple1<IEntity> ctx = Tuple1<IEntity>.Cast(context);
		if (!source || !ctx || !ctx.param1)
			return;

		string reason;
		ROOT_AE3_ConnectionComponent.ConnectPower(ctx.param1, source, reason);
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(ctx.param1);
		if (p)
			p.ApplyRestoredState();
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnRouter(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		IEntity router = IEntity.Cast(instance);
		Tuple4<IEntity, bool, float, string> ctx = Tuple4<IEntity, bool, float, string>.Cast(context);
		if (!router || !ctx || !ctx.param1)
			return;

		string reason;
		if (ctx.param2)
		{
			ROOT_AE3_ConnectionComponent.ConnectNetwork(ctx.param1, router, reason, ctx.param3, ctx.param4);
			return;
		}

		ROOT_AE3_NetworkComponent client = ROOT_AE3_NetworkComponent.Of(ctx.param1);
		ROOT_AE3_NetworkComponent r = ROOT_AE3_NetworkComponent.Of(router);
		if (client && r)
			ROOT_AE3_NetworkSystem.Get().Connect(client, r, r.GetPassword(), reason, true);
	}
}
