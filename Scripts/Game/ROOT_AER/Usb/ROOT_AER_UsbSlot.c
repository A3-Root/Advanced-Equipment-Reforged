//! Flash drive plugged into a computer port. Holds the drive state while the drive item does not exist.
class ROOT_AER_UsbSlot
{
	ref ROOT_AER_Filesystem m_Fs;
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
	ROOT_AER_Filesystem GetFilesystem()
	{
		return m_Fs;
	}
}
