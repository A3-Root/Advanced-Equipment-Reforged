[ComponentEditorProps(category: "Advanced Equipment Reforged", description: "USB flash drive item with its own filesystem")]
class ROOT_AER_UsbDriveComponentClass : ScriptComponentClass
{
}

//! Flash drive. The filesystem lives on the authority and travels with the item.
class ROOT_AER_UsbDriveComponent : ScriptComponent
{
	[Attribute("Flash Drive", desc: "Label shown by lsusb", category: "USB")]
	protected string m_sLabel;

	[Attribute("8192", desc: "Capacity in MB", params: "1 1048576 1", category: "USB")]
	protected int m_iCapacity;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Prop shown in a USB port", params: "et", category: "USB")]
	protected ResourceName m_sPortProp;

	[Attribute(desc: "Files and directories on the drive", category: "USB")]
	protected ref array<ref ROOT_AER_FileEntry> m_aFiles;

	protected ref ROOT_AER_Filesystem m_Fs;

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_UsbDriveComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AER_UsbDriveComponent.Cast(ent.FindComponent(ROOT_AER_UsbDriveComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. Created on first use.
	ROOT_AER_Filesystem GetFilesystem()
	{
		if (!m_Fs)
		{
			m_Fs = new ROOT_AER_Filesystem();
			m_Fs.m_Root.m_iPerms = 63;
			m_Fs.m_fCapacityMB = m_iCapacity;
			if (m_aFiles)
			{
				string err;
				foreach (ROOT_AER_FileEntry f : m_aFiles)
				{
					if (f)
						f.Apply(m_Fs, err);
				}
			}
		}

		return m_Fs;
	}

	//------------------------------------------------------------------------------------------------
	void SetFilesystem(ROOT_AER_Filesystem fs)
	{
		m_Fs = fs;
		if (m_Fs)
			m_Fs.m_fCapacityMB = m_iCapacity;
	}

	//------------------------------------------------------------------------------------------------
	string GetLabel()
	{
		return m_sLabel;
	}

	//------------------------------------------------------------------------------------------------
	void SetLabel(string label)
	{
		m_sLabel = ROOT_AER_Codec.Clean(label);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: label after a rename; empty name = "Flash Drive".
	void OnRenamed(string name)
	{
		if (name.IsEmpty())
			SetLabel("Flash Drive");
		else
			SetLabel(name);
	}

	//------------------------------------------------------------------------------------------------
	int GetCapacity()
	{
		return m_iCapacity;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetPortProp()
	{
		return m_sPortProp;
	}

	//------------------------------------------------------------------------------------------------
	//! State of this drive for a computer port.
	ROOT_AER_UsbSlot ToSlot()
	{
		ROOT_AER_UsbSlot slot = new ROOT_AER_UsbSlot();
		slot.m_Fs = GetFilesystem();
		slot.m_sLabel = m_sLabel;
		slot.m_iCapacity = m_iCapacity;
		EntityPrefabData pd = GetOwner().GetPrefabData();
		if (pd)
			slot.m_sPrefab = pd.GetPrefabName();

		return slot;
	}

	//------------------------------------------------------------------------------------------------
	void FromSlot(ROOT_AER_UsbSlot slot)
	{
		string prefabLabel = m_sLabel;
		m_Fs = slot.m_Fs;
		m_sLabel = slot.m_sLabel;
		m_iCapacity = slot.m_iCapacity;
		ROOT_AER_DeviceComponent d = ROOT_AER_DeviceComponent.Of(GetOwner());
		if (d && m_sLabel != prefabLabel)
			d.SetDeviceName(m_sLabel);
	}
}
