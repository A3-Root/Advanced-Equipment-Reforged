//! Persistence of the armaOS state (ROOT_AER_SaveLoad.Pack) in vanilla persistence saves. Server only.
class ROOT_AER_ComputerSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return ROOT_AER_ComputerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull SaveContext context)
	{
		ROOT_AER_ComputerComponent c = ROOT_AER_ComputerComponent.Cast(component);
		if (!c || !c.GetFilesystem())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValue("state", ROOT_AER_SaveLoad.Pack(c));
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull LoadContext context)
	{
		ROOT_AER_ComputerComponent c = ROOT_AER_ComputerComponent.Cast(component);
		int version;
		string state;
		context.ReadValue("version", version);
		if (!c || !context.ReadValue("state", state))
			return false;

		return ROOT_AER_SaveLoad.Unpack(c, state);
	}
}
