//! Persistence of the AE3 System module settings (every ROOT_AE3_Settings.Keys value). Server only.
class ROOT_AE3_SystemModuleSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return ROOT_AE3_SystemModuleComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull SaveContext context)
	{
		ROOT_AE3_SystemModuleComponent m = ROOT_AE3_SystemModuleComponent.Cast(component);
		if (!m || !m.GetSettings())
			return ESerializeResult.DEFAULT;

		ROOT_AE3_Settings s = m.GetSettings();
		array<string> numbers = {};
		array<string> texts = {};
		ROOT_AE3_Settings.Keys(numbers, texts);
		context.WriteValue("version", 1);
		foreach (string key : numbers)
		{
			float value;
			if (s.GetNumber(key, value))
				context.WriteValue("n_" + key, value);
		}

		foreach (string key : texts)
		{
			string text;
			if (s.GetText(key, text))
				context.WriteValue("t_" + key, text);
		}

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull LoadContext context)
	{
		ROOT_AE3_SystemModuleComponent m = ROOT_AE3_SystemModuleComponent.Cast(component);
		if (!m || !m.GetSettings())
			return false;

		ROOT_AE3_Settings s = m.GetSettings();
		array<string> numbers = {};
		array<string> texts = {};
		ROOT_AE3_Settings.Keys(numbers, texts);
		int version;
		context.ReadValue("version", version);
		foreach (string key : numbers)
		{
			float value;
			if (context.ReadValue("n_" + key, value))
				s.SetNumber(key, value);
		}

		foreach (string key : texts)
		{
			string text;
			if (context.ReadValue("t_" + key, text))
				s.SetText(key, text);
		}

		if (m == ROOT_AE3_SystemModuleComponent.GetActive())
			ROOT_AE3_SettingsManager.Broadcast();

		return true;
	}
}
