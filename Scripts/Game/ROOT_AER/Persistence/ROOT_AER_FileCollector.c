//! FileIO.FindFiles callback target.
class ROOT_AER_FileCollector
{
	ref array<string> m_aFiles = {};

	//------------------------------------------------------------------------------------------------
	void Add(string fileName, FileAttribute attributes = 0, string filesystem = string.Empty)
	{
		m_aFiles.Insert(fileName);
	}
}
