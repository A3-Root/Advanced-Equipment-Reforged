//! Pours fuel from a carried fuel container (item with a FuelManagerComponent) into a generator; the
//! container is drained by the amount poured.
class ROOT_AE3_RefuelAction : ROOT_AE3_UserActionBase
{

	//------------------------------------------------------------------------------------------------
	protected IEntity FindCan(IEntity user)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return null;

		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity item : items)
		{
			FuelManagerComponent fuel = FuelManagerComponent.Cast(item.FindComponent(FuelManagerComponent));
			if (fuel && fuel.GetTotalFuel() > 0.01 && HasDiesel(fuel))
				return item;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasDiesel(FuelManagerComponent fuel)
	{
		array<BaseFuelNode> nodes = {};
		fuel.GetFuelNodesList(nodes);
		foreach (BaseFuelNode node : nodes)
		{
			if (node.GetFuelType() == EFuelType.Diesel && node.GetFuel() > 0.01)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (ROOT_AE3_Util.IsDestroyed(GetOwner()))
			return false;

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		return p && p.GetFuelCapacity() > 0 && IsPlaced(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		if (p.GetFuel() >= p.GetFuelCapacity() - 0.01)
		{
			SetCannotPerformReason("Tank full");
			return false;
		}

		bool due = ScanDue(user);
		if (!CachedScan(due, due && FindCan(user) != null))
		{
			SetCannotPerformReason("No jerry can");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!IsAuthority(pOwnerEntity) || !InReach(pOwnerEntity, pUserEntity))
			return;

		IEntity can = FindCan(pUserEntity);
		if (!can)
			return;

		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(pOwnerEntity);
		float space = p.GetFuelCapacity() - p.GetFuel();
		FuelManagerComponent fuel = FuelManagerComponent.Cast(can.FindComponent(FuelManagerComponent));
		if (fuel)
		{
			array<BaseFuelNode> nodes = {};
			fuel.GetFuelNodesList(nodes);
			float poured;
			foreach (BaseFuelNode node : nodes)
			{
				if (node.GetFuelType() != EFuelType.Diesel)
					continue;

				float take = Math.Min(node.GetFuel(), space - poured);
				if (take <= 0)
					continue;

				node.SetFuel(node.GetFuel() - take);
				poured += take;
			}

			p.SetFuel(p.GetFuel() + poured);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		ROOT_AE3_PowerComponent p = ROOT_AE3_PowerComponent.Of(GetOwner());
		outName = string.Format("Refuel (%1 / %2 L)", Math.Round(p.GetFuel()), Math.Round(p.GetFuelCapacity()));
		return true;
	}
}
