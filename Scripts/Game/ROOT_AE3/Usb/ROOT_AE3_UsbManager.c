//! Server: plugging flash drives into computer ports and taking them out again.
//! Plugging moves the drive state into the port and removes the item; ejecting spawns the drive with its state.
class ROOT_AE3_UsbManager
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
			if (ROOT_AE3_UsbDriveComponent.Of(item))
				return item;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	static bool Insert(ROOT_AE3_ComputerComponent c, IEntity character, out string reason)
	{
		IEntity item = FindDrive(character);
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

		ROOT_AE3_UsbDriveComponent drive = ROOT_AE3_UsbDriveComponent.Of(item);
		ROOT_AE3_UsbSlot slot = drive.ToSlot();
		c.SetUsbDrive(port, slot);
		slot.m_sPropPrefab = drive.GetPortProp();
		slot.m_Prop = SpawnProp(c, port, slot.m_sPropPrefab);
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv || !inv.TryDeleteItem(item))
			SCR_EntityHelper.DeleteEntityAndChildren(item);

		ROOT_AE3_Sound.Play(c.GetOwner(), ROOT_AE3_Sound.USB);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static IEntity SpawnProp(ROOT_AE3_ComputerComponent c, int port, ResourceName prefab)
	{
		if (prefab.IsEmpty())
			return null;

		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return null;

		EntitySpawnParams p = new EntitySpawnParams();
		p.TransformMode = ETransformMode.WORLD;
		c.GetOwner().GetWorldTransform(p.Transform);
		p.Transform[3] = c.GetOwner().CoordToParent(c.GetUsbPortPos(port));
		IEntity prop = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), p);
		if (prop)
			c.GetOwner().AddChild(prop, -1, EAddChildFlags.AUTO_TRANSFORM);

		return prop;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns the drive of a port at the computer and puts it into the character's inventory when possible.
	static bool Eject(ROOT_AE3_ComputerComponent c, int port, IEntity character, out string reason)
	{
		ROOT_AE3_UsbSlot slot = c.GetUsbDrive(port);
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

		ROOT_AE3_UsbDriveComponent drive = ROOT_AE3_UsbDriveComponent.Of(item);
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

		ROOT_AE3_Sound.Play(c.GetOwner(), ROOT_AE3_Sound.USB);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Ejects every port onto the ground (computer picked up or removed).
	static void EjectAll(ROOT_AE3_ComputerComponent c, IEntity character = null)
	{
		string reason;
		for (int i = 0; i < c.GetUsbPortCount(); i++)
		{
			if (c.GetUsbDrive(i))
				Eject(c, i, character, reason);
		}
	}
}
