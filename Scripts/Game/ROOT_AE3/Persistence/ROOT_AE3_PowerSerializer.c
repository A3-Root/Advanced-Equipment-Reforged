//! Persistence of power state, battery level and fuel. Server only.
class ROOT_AE3_PowerSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return ROOT_AE3_PowerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull SaveContext context)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Cast(component);
		if (!p)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValue("state", p.GetState());
		context.WriteValue("level", p.GetLevel());
		context.WriteValue("fuel", p.GetFuel());
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull LoadContext context)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Cast(component);
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

		return true;
	}
}
