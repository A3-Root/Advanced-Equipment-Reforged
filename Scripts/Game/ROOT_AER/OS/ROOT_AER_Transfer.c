//! Simulated transfer durations: size / speed of the medium, capped by TransferMaxDuration.
class ROOT_AER_Transfer
{
	static const string LOCAL = "local";
	static const string USB = "usb";
	static const string NETWORK = "network";

	//------------------------------------------------------------------------------------------------
	//! Medium of a copy between two absolute paths.
	static string MediumOf(string src, string dst)
	{
		if (src.StartsWith("/mnt/usb") || dst.StartsWith("/mnt/usb"))
			return USB;

		return LOCAL;
	}

	//------------------------------------------------------------------------------------------------
	//! Seconds; 0 when below half a second or the speed is 0. speedKBps < 0 = setting of the medium.
	static float Duration(int bytes, string medium, float speedKBps = -1)
	{
		ROOT_AER_Settings s = ROOT_AER_SettingsManager.Get();
		float speed = speedKBps;
		if (speed < 0)
		{
			switch (medium)
			{
				case USB: speed = s.m_fTransferSpeedUsb; break;
				case NETWORK: speed = s.m_fTransferSpeedNetwork; break;
				default: speed = s.m_fTransferSpeedLocal;
			}
		}

		if (speed <= 0)
			return 0;

		float d = (bytes / 1024.0) / speed;
		if (s.m_fTransferMaxDuration > 0)
			d = Math.Min(d, s.m_fTransferMaxDuration);

		if (d < 0.5)
			return 0;

		return d;
	}
}
