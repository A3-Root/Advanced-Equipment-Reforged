//! Persistence of the armaOS state (ROOT_AE3_SaveLoad.Pack) in vanilla persistence saves. Server only.
class ROOT_AE3_ComputerSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return ROOT_AE3_ComputerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull SaveContext context)
	{
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Cast(component);
		if (!c || !c.GetFilesystem())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValue("state", ROOT_AE3_SaveLoad.Pack(c));
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull LoadContext context)
	{
		ROOT_AE3_ComputerComponent c = ROOT_AE3_ComputerComponent.Cast(component);
		int version;
		string state;
		context.ReadValue("version", version);
		if (!c || !context.ReadValue("state", state))
			return false;

		return ROOT_AE3_SaveLoad.Unpack(c, state);
	}
}
