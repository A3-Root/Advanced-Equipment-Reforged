[ComponentEditorProps(category: "Advanced Equipment", description: "Network cable item: plugged between a device and a router")]
class ROOT_AE3_CableComponentClass : ScriptComponentClass
{
}

//! Network cable item. Plugging it in removes the item and stores its prefab and length with the link;
//! unplugging gives it back (ROOT_AE3_ConnectionComponent).
class ROOT_AE3_CableComponent : ScriptComponent
{
	[Attribute("15", desc: "Cable length in m", params: "1 100 1")]
	protected float m_fLength;

	//------------------------------------------------------------------------------------------------
	static ROOT_AE3_CableComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AE3_CableComponent.Cast(ent.FindComponent(ROOT_AE3_CableComponent));
	}

	//------------------------------------------------------------------------------------------------
	float GetLength()
	{
		return m_fLength;
	}

	//------------------------------------------------------------------------------------------------
	//! Cable items in a character's inventory.
	static void Carried(IEntity character, notnull array<IEntity> outItems)
	{
		outItems.Clear();
		if (!character)
			return;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			if (Of(item))
				outItems.Insert(item);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Distinct lengths of the carried cables, ascending.
	static void CarriedLengths(IEntity character, notnull array<float> lengths)
	{
		lengths.Clear();
		array<IEntity> items = {};
		Carried(character, items);
		foreach (IEntity item : items)
		{
			float l = Of(item).GetLength();
			if (!lengths.Contains(l))
				lengths.Insert(l);
		}

		lengths.Sort();
	}

	//------------------------------------------------------------------------------------------------
	//! Carried cable of that length (any length when length <= 0: the longest).
	static IEntity FindCarried(IEntity character, float length)
	{
		array<IEntity> items = {};
		Carried(character, items);
		IEntity best;
		float bestLen = -1;
		foreach (IEntity item : items)
		{
			float l = Of(item).GetLength();
			if (length > 0 && !float.AlmostEqual(l, length))
				continue;

			if (l > bestLen)
			{
				best = item;
				bestLen = l;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: removes a cable item from the character's inventory (or the world).
	static void Consume(IEntity character, IEntity item)
	{
		if (!item)
			return;

		SCR_InventoryStorageManagerComponent inv;
		if (character)
			inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));

		if (!inv || !inv.TryDeleteItem(item))
			SCR_EntityHelper.DeleteEntityAndChildren(item);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority: spawns a cable item and puts it into the character's inventory when possible (character null and
	//! the device carried: the carrier). Otherwise it lies at the device, or at the feet of the carrier.
	static IEntity Give(ResourceName prefab, IEntity device, IEntity character)
	{
		if (prefab.IsEmpty() || !device)
			return null;

		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return null;

		IEntity carrier = ROOT_AE3_Util.Carrier(device);
		if (!character)
			character = carrier;

		vector pos = device.GetOrigin() + "0 0.02 0";
		if (carrier)
		{
			pos = carrier.GetOrigin() + carrier.VectorToParent("0 0 0.4");
			pos[1] = Math.Max(GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]), carrier.GetOrigin()[1]) + 0.02;
		}

		EntitySpawnParams p = new EntitySpawnParams();
		p.TransformMode = ETransformMode.WORLD;
		Math3D.MatrixIdentity4(p.Transform);
		p.Transform[3] = pos;
		IEntity item = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), p);
		if (!item || !character)
			return item;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inv)
			inv.TryInsertItem(item);

		return item;
	}
}
