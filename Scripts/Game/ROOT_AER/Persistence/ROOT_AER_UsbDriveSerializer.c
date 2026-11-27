//! Persistence of flash drive content, label and custom device name. Server only.
class ROOT_AER_UsbDriveSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return ROOT_AER_UsbDriveComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull SaveContext context)
	{
		ROOT_AER_UsbDriveComponent d = ROOT_AER_UsbDriveComponent.Cast(component);
		if (!d)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 2);
		ROOT_AER_DeviceComponent dev = ROOT_AER_DeviceComponent.Of(owner);
		if (dev)
			context.WriteValue("name", dev.GetCustomName());

		context.WriteValue("label", d.GetLabel());
		context.WriteValue("fs", d.GetFilesystem().Serialize());
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull LoadContext context)
	{
		ROOT_AER_UsbDriveComponent d = ROOT_AER_UsbDriveComponent.Cast(component);
		if (!d)
			return false;

		int version;
		string label, fs;
		context.ReadValue("version", version);
		context.ReadValue("label", label);
		if (!context.ReadValue("fs", fs))
			return false;

		d.SetLabel(label);
		d.SetFilesystem(ROOT_AER_Filesystem.Deserialize(fs));
		string name;
		ROOT_AER_DeviceComponent dev = ROOT_AER_DeviceComponent.Of(owner);
		if (version >= 2 && dev && context.ReadValue("name", name))
			dev.SetDeviceName(name);

		return true;
	}
}
