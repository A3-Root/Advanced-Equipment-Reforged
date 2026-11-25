//! armaOS program 'lsusb': Lists the available (usb) interfaces.
class ROOT_AE3_Cmd_Lsusb : ROOT_AE3_Command
{
	//------------------------------------------------------------------------------------------------
	override string GetName()
	{
		return "lsusb";
	}

	//------------------------------------------------------------------------------------------------
	override string GetPath()
	{
		return "/bin/lsusb";
	}

	//------------------------------------------------------------------------------------------------
	override string GetDescription()
	{
		return "Lists the available (usb) interfaces.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetManual()
	{
		return "Usage lsusb: 'lsusb' lists usb interfaces.";
	}

	//------------------------------------------------------------------------------------------------
	//! Port index of "usbN", -1 when invalid.
	static int PortOf(string iface)
	{
		if (!iface.StartsWith("usb") || iface.Length() != 4 || !iface.IsDigitAt(3))
			return -1;

		return iface.Substring(3, 1).ToInt();
	}

	//------------------------------------------------------------------------------------------------
	override void Run(ROOT_AE3_ShellContext ctx, array<string> args)
	{
		int n = ctx.m_Exec.GetUsbPortCount();
		if (n == 0)
		{
			ctx.Out("No usb interfaces.");
			return;
		}

		for (int i = 0; i < n; i++)
		{
			string name = "usb" + i.ToString();
			ROOT_AE3_UsbSlot drive = ctx.m_Exec.GetUsbDrive(i);
			if (!drive)
			{
				ctx.Out(name + ": empty");
				continue;
			}

			string state = "not mounted";
			if (ctx.m_Exec.IsMounted(i))
				state = "mounted at /mnt/" + name;

			ctx.Out(string.Format("%1: %2 (%3 used) - %4", name, drive.GetLabel(), ROOT_AE3_Util.FormatSize(drive.GetFilesystem().m_Root.GetSize()), state));
		}
	}
}
