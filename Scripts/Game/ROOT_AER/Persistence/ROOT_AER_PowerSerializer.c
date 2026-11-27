//! Persistence of power state, battery level, fuel and custom device name. Server only.
class ROOT_AER_PowerSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return ROOT_AER_PowerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull SaveContext context)
	{
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Cast(component);
		if (!p)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 2);
		ROOT_AER_DeviceComponent dev = ROOT_AER_DeviceComponent.Of(owner);
		if (dev)
			context.WriteValue("name", dev.GetCustomName());

		context.WriteValue("state", p.GetState());
		context.WriteValue("level", p.GetLevel());
		context.WriteValue("fuel", p.GetFuel());
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull LoadContext context)
	{
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Cast(component);
		if (!p)
			return false;

		int version, state;
		float level, fuel;
		context.ReadValue("version", version);
		context.ReadValue("state", state);
		context.ReadValue("level", level);
		context.ReadValue("fuel", fuel);
		p.SetLevel(level);
		p.SetFuel(fuel);
		p.SetRestoredState(state);
		string name;
		ROOT_AER_DeviceComponent dev = ROOT_AER_DeviceComponent.Of(owner);
		if (version >= 2 && dev && context.ReadValue("name", name))
			dev.SetDeviceName(name);

		return true;
	}
}
