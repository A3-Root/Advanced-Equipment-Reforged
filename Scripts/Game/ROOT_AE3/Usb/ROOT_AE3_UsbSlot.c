//! Flash drive plugged into a computer port. Holds the drive state while the drive item does not exist.
class ROOT_AE3_UsbSlot
{
	ref ROOT_AE3_Filesystem m_Fs;
	string m_sLabel;
	int m_iCapacity;
	ResourceName m_sPrefab;
	ResourceName m_sPropPrefab;
	IEntity m_Prop;

	//------------------------------------------------------------------------------------------------
	string GetLabel()
	{
		return m_sLabel;
	}

	//------------------------------------------------------------------------------------------------
	ROOT_AE3_Filesystem GetFilesystem()
	{
		return m_Fs;
	}
}
