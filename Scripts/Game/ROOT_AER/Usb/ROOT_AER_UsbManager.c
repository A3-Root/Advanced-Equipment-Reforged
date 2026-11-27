//! Server: plugging flash drives into computer ports and taking them out again.
//! Plugging moves the drive state into the port and removes the item; ejecting spawns the drive with its state.
class ROOT_AER_UsbManager
{
	//------------------------------------------------------------------------------------------------
	//! First flash drive in a character's inventory.
	static IEntity FindDrive(IEntity character)
	{
		if (!character)
			return null;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return null;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			if (ROOT_AER_UsbDriveComponent.Of(item))
				return item;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Flash drives in the inventory of the character.
	static void Drives(IEntity character, notnull array<IEntity> found)
	{
		found.Clear();
		if (!character)
			return;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			if (ROOT_AER_UsbDriveComponent.Of(item))
				found.Insert(item);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Replication id of a drive item as a number, -1 without one.
	static int DriveId(IEntity item)
	{
		if (!item)
			return -1;

		RplComponent rpl = RplComponent.Cast(item.FindComponent(RplComponent));
		if (!rpl)
			return -1;

		int id = rpl.Id();
		return id;
	}

	//------------------------------------------------------------------------------------------------
	//! Carried drive with that DriveId, null when the character does not carry it.
	static IEntity FindDriveById(IEntity character, int id)
	{
		array<IEntity> drives = {};
		Drives(character, drives);
		foreach (IEntity d : drives)
		{
			if (DriveId(d) == id)
				return d;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Label and size shown when a drive is chosen.
	static string DriveName(IEntity item)
	{
		ROOT_AER_UsbDriveComponent d = ROOT_AER_UsbDriveComponent.Of(item);
		if (!d)
			return "Flash drive";

		int mb = d.GetCapacity();
		string size = mb.ToString() + " MB";
		if (mb >= 1024)
			size = ROOT_AER_Util.FormatFloat(mb / 1024.0, 1) + " GB";

		string name = d.GetLabel();
		ROOT_AER_DeviceComponent dev = ROOT_AER_DeviceComponent.Of(item);
		if (dev && !dev.GetCustomName().IsEmpty())
			name = dev.GetCustomName();

		return name + " (" + size + ")";
	}

	//------------------------------------------------------------------------------------------------
	//! Plugs drive (or the first carried drive when null) of the character into the first free port.
	static bool Insert(ROOT_AER_ComputerComponent c, IEntity character, out string reason, IEntity drive = null)
	{
		IEntity item = drive;
		if (!item)
			item = FindDrive(character);

		if (!item)
		{
			reason = "No flash drive in the inventory.";
			return false;
		}

		int port = c.FreeUsbPort();
		if (port < 0)
		{
			reason = "No free USB port.";
			return false;
		}

		ROOT_AER_UsbDriveComponent comp = ROOT_AER_UsbDriveComponent.Of(item);
		ROOT_AER_UsbSlot slot = comp.ToSlot();
		c.SetUsbDrive(port, slot);
		slot.m_sPropPrefab = comp.GetPortProp();
		slot.m_Prop = SpawnProp(c, port, slot.m_sPropPrefab);
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv || !inv.TryDeleteItem(item))
			SCR_EntityHelper.DeleteEntityAndChildren(item);

		ROOT_AER_Sound.Play(c.GetOwner(), ROOT_AER_Sound.USB);
		return true;
	}

	protected static const float PROP_BODY_FRONT = 0.0225;	//!< Stick model: origin to the front of the body (plug side, +X)
	protected static const float PROP_HALF_HEIGHT = 0.004;	//!< Stick model: origin to the plug centre (up)
	protected static const float PORT_HALF_DEPTH = 0.001;	//!< Port box: centre to the outer face

	//------------------------------------------------------------------------------------------------
	//! Stick prop in port: ports sit on the left (-X) or right (+X) side of the model; the plug points into
	//! the case and the body starts at the outer face of the port.
	static IEntity SpawnProp(ROOT_AER_ComputerComponent c, int port, ResourceName prefab)
	{
		if (prefab.IsEmpty())
			return null;

		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return null;

		vector pos = c.GetUsbPortPos(port);
		float side = 1;
		float yaw = 180;
		if (pos[0] < 0)
		{
			side = -1;
			yaw = 0;
		}

		vector local[4];
		Math3D.AnglesToMatrix(Vector(yaw, 0, 0), local);
		local[3] = Vector(pos[0] + side * (PORT_HALF_DEPTH + PROP_BODY_FRONT), pos[1] - PROP_HALF_HEIGHT, pos[2]);
		vector world[4];
		c.GetOwner().GetWorldTransform(world);
		EntitySpawnParams p = new EntitySpawnParams();
		p.TransformMode = ETransformMode.WORLD;
		Math3D.MatrixMultiply4(world, local, p.Transform);
		IEntity prop = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), p);
		if (prop)
			c.GetOwner().AddChild(prop, -1, EAddChildFlags.AUTO_TRANSFORM | EAddChildFlags.RECALC_LOCAL_TRANSFORM);

		return prop;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns the drive of a port at the computer and puts it into the character's inventory when possible.
	static bool Eject(ROOT_AER_ComputerComponent c, int port, IEntity character, out string reason)
	{
		ROOT_AER_UsbSlot slot = c.GetUsbDrive(port);
		if (!slot)
		{
			reason = "No flash drive in that port.";
			return false;
		}

		Resource res = Resource.Load(slot.m_sPrefab);
		if (!res || !res.IsValid())
		{
			reason = "Flash drive prefab missing.";
			return false;
		}

		EntitySpawnParams p = new EntitySpawnParams();
		p.TransformMode = ETransformMode.WORLD;
		c.GetOwner().GetWorldTransform(p.Transform);
		p.Transform[3] = c.GetOwner().GetOrigin() + "0 0.15 0";
		IEntity item = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), p);
		if (!item)
		{
			reason = "Flash drive could not be created.";
			return false;
		}

		ROOT_AER_UsbDriveComponent drive = ROOT_AER_UsbDriveComponent.Of(item);
		if (drive)
			drive.FromSlot(slot);

		if (slot.m_Prop)
			SCR_EntityHelper.DeleteEntityAndChildren(slot.m_Prop);

		c.SetUsbDrive(port, null);
		if (character)
		{
			SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
			if (inv)
				inv.TryInsertItem(item);
		}

		ROOT_AER_Sound.Play(c.GetOwner(), ROOT_AER_Sound.USB_OUT);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Ejects every port onto the ground (computer picked up or removed).
	static void EjectAll(ROOT_AER_ComputerComponent c, IEntity character = null)
	{
		string reason;
		for (int i = 0; i < c.GetUsbPortCount(); i++)
		{
			if (c.GetUsbDrive(i))
				Eject(c, i, character, reason);
		}
	}
}
