[ComponentEditorProps(category: "Advanced Equipment Reforged", description: "Laptop lid: open / closed model; a closed laptop goes to standby and cannot be used")]
class ROOT_AER_LidComponentClass : ScriptComponentClass
{
}

//! Lid state of a laptop. Replicated; every machine swaps the model locally.
class ROOT_AER_LidComponent : ScriptComponent
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Model with open lid", params: "xob")]
	protected ResourceName m_sOpenModel;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Model with closed lid", params: "xob")]
	protected ResourceName m_sClosedModel;

	[Attribute(desc: "Material remaps applied to the swapped model: pairs of source material name and .emat")]
	protected ref array<string> m_aRemap;

	[Attribute("1", desc: "Open at start"), RplProp(onRplName: "ApplyModel")]
	protected bool m_bOpen;

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (GetGame().InPlayMode())
			GetGame().GetCallqueue().CallLater(ApplyModel, 100, false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(ApplyModel);
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	static ROOT_AER_LidComponent Of(IEntity ent)
	{
		if (!ent)
			return null;

		return ROOT_AER_LidComponent.Cast(ent.FindComponent(ROOT_AER_LidComponent));
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpen()
	{
		return m_bOpen;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority.
	void SetOpen(bool open)
	{
		if (m_bOpen == open)
			return;

		m_bOpen = open;
		Replication.BumpMe();
		ApplyModel();
		if (open)
			return;

		ROOT_AER_ComputerComponent c = ROOT_AER_ComputerComponent.Of(GetOwner());
		ROOT_AER_PowerComponent p = ROOT_AER_PowerComponent.Of(GetOwner());
		if (c)
			ROOT_AER_Manager.Get().EndSessionsOf(ROOT_AER_DeviceComponent.Of(GetOwner()));

		string reason;
		if (p && p.IsOn())
			p.Standby(reason);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyModel()
	{
		ResourceName model = m_sClosedModel;
		if (m_bOpen)
			model = m_sOpenModel;

		if (model.IsEmpty())
			return;

		Resource res = Resource.Load(model);
		if (!res || !res.IsValid())
			return;

		string remap;
		if (m_aRemap)
		{
			for (int i = 0; i + 1 < m_aRemap.Count(); i += 2)
			{
				remap += string.Format("$remap '%1' '%2';", m_aRemap[i], m_aRemap[i + 1]);
			}
		}

		GetOwner().SetObject(res.GetResource().ToVObject(), remap);
		Physics phys = GetOwner().GetPhysics();
		if (phys)
			phys.UpdateGeometries();

		GetOwner().Update();
		ROOT_AER_ScreenComponent screen = ROOT_AER_ScreenComponent.Of(GetOwner());
		if (screen)
			screen.Rebind();
	}
}
