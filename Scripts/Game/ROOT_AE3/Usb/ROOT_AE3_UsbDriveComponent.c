[ComponentEditorProps(category: "Advanced Equipment", description: "USB flash drive item with its own filesystem")]
class ROOT_AE3_UsbDriveComponentClass : ScriptComponentClass
{
}

//! Flash drive. The filesystem lives on the authority and travels with the item.
class ROOT_AE3_UsbDriveComponent : ScriptComponent
{
	[Attribute("Flash Drive", desc: "Label shown by lsusb", category: "USB")]
	protected string m_sLabel;

	[Attribute("8192", desc: "Capacity in MB", params: "1 1048576 1", category: "USB")]
	protected int m_iCapacity;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Prop shown in a USB port", params: "et", category: "USB")]
	protected ResourceName m_sPortProp;

	[Attribute(desc: "Files and directories on the drive", category: "USB")]
	protected ref array<ref ROOT_AE3_FileEntry> m_aFiles;

	protected ref ROOT_AE3_Filesystem m_Fs;

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_UsbDriveComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AE3_UsbDriveComponent.Cast(ent.FindComponent(ROOT_AE3_UsbDriveComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Authority. Created on first use.
	ROOT_AE3_Filesystem GetFilesystem()
	{
		if (!m_Fs)
		{
			m_Fs = new ROOT_AE3_Filesystem();
			m_Fs.m_Root.m_iPerms = 63;
			m_Fs.m_fCapacityMB = m_iCapacity;
			if (m_aFiles)
			{
				string err;
				foreach (ROOT_AE3_FileEntry f : m_aFiles)
				{
					if (f)
						f.Apply(m_Fs, err);
				}
			}
		}

		return m_Fs;
	}

	//------------------------------------------------------------------------------------------------
	void SetFilesystem(ROOT_AE3_Filesystem fs)
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
		m_sLabel = ROOT_AE3_Codec.Clean(label);
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
	ROOT_AE3_UsbSlot ToSlot()
	{
		ROOT_AE3_UsbSlot slot = new ROOT_AE3_UsbSlot();
		slot.m_Fs = GetFilesystem();
		slot.m_sLabel = m_sLabel;
		slot.m_iCapacity = m_iCapacity;
		EntityPrefabData pd = GetOwner().GetPrefabData();
		if (pd)
			slot.m_sPrefab = pd.GetPrefabName();

		return slot;
	}

	//------------------------------------------------------------------------------------------------
	void FromSlot(ROOT_AE3_UsbSlot slot)
	{
		m_Fs = slot.m_Fs;
		m_sLabel = slot.m_sLabel;
		m_iCapacity = slot.m_iCapacity;
	}
}
