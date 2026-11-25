//! Headless resource helper.
//! -ae3List=<path|path...>: registers each listed absolute path.
//! -ae3Rebuild=<path|path...>: builds runtime resources (.xob from .fbx, .edds from .png).
//! -ae3Resave=<path|path...>: loads and re-saves each listed registered .et / .conf / .layout so Workbench writes its IDs.
//! -ae3KeepOpen=1: stays open so asynchronous imports finish (the caller ends the process).
//! Command line: -wbmodule=ScriptEditor -plugin=ROOT_AE3_RegisterResourcesPlugin -ae3List=<path|path>
[WorkbenchPluginAttribute(name: "AE3 Register Resources", description: "Register or re-save listed files", wbModules: {"ResourceManager", "ScriptEditor"}, awesomeFontCode: 0xF0C7)]
class ROOT_AE3_RegisterResourcesPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	protected bool GetArg(string name, out string value)
	{
		ScriptEditor se = Workbench.GetModule(ScriptEditor);
		if (se && se.GetCmdLine(name, value))
		{
			value.Replace("\"", "");
			return true;
		}

		ResourceManager rm = Workbench.GetModule(ResourceManager);
		if (rm && rm.GetCmdLine(name, value))
		{
			value.Replace("\"", "");
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Paths separated by '|' (passed on the command line, no file access).
	protected array<string> ReadList(string list)
	{
		array<string> lines = {};
		list.Split("|", lines, true);
		return lines;
	}

	//------------------------------------------------------------------------------------------------
	//! Prints the Game Master placing registries of the edit mode as Workbench resolves them, and the UI info of each mod prefab.
	protected void InspectPlacing()
	{
		Resource cat = BaseContainerTools.LoadContainer("{5F7EC52FC40A03E2}Configs/EntityCatalog/US/InventoryItems_EntityCatalog_US.conf");
		if (cat && cat.IsValid())
		{
			BaseContainerList lists = cat.GetResource().ToBaseContainer().GetObjectArray("m_aMultiLists");
			for (int l = 0; l < lists.Count(); l++)
			{
				string ident;
				lists.Get(l).Get("m_sIdentifier", ident);
				BaseContainerList ents = lists.Get(l).GetObjectArray("m_aEntities");
				int cnt = 0;
				if (ents)
					cnt = ents.Count();

				PrintFormat("[AE3] inspect: catalog US list %1 '%2' entities=%3", l, ident, cnt);
			}
		}

		Resource res = BaseContainerTools.LoadContainer("{59EF8ECAE1DCD417}Prefabs/Editor/Modes/EditorModeEdit.et");
		if (!res || !res.IsValid())
		{
			Print("[AE3] inspect: edit mode prefab not loaded", LogLevel.WARNING);
			return;
		}

		IEntitySource src = res.GetResource().ToEntitySource();
		for (int i = 0, n = src.GetComponentCount(); i < n; i++)
		{
			IEntityComponentSource comp = src.GetComponent(i);
			if (comp.GetClassName() != "SCR_PlacingEditorComponent")
				continue;

			BaseContainerList regs = comp.GetObjectArray("m_Registries");
			PrintFormat("[AE3] inspect: registries %1", regs.Count());
			for (int r = 0; r < regs.Count(); r++)
			{
				array<ResourceName> prefabs = {};
				regs.Get(r).Get("m_Prefabs", prefabs);
				string first;
				if (!prefabs.IsEmpty())
					first = prefabs[0];

				PrintFormat("[AE3] inspect: registry %1 %2 prefabs=%3 first=%4", r, regs.Get(r).GetClassName(), prefabs.Count(), first);
				foreach (ResourceName prefab : prefabs)
				{
					if (!prefab.Contains("ROOT_AE3"))
						continue;

					SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.ExtractEditableUIInfoFromPrefab(prefab);
					string name = "NULL";
					if (info)
						name = info.GetName();

					PrintFormat("[AE3] inspect: %1 -> %2", prefab, name);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Prints inventory size, volume and storage limits of the given prefabs as Workbench resolves them.
	protected void InspectItems(string list)
	{
		foreach (string prefab : ReadList(list))
		{
			Resource res = BaseContainerTools.LoadContainer(prefab);
			if (!res || !res.IsValid())
			{
				Print("[AE3] item: not loaded " + prefab, LogLevel.WARNING);
				continue;
			}

			IEntitySource src = res.GetResource().ToEntitySource();
			string classes;
			for (int j = 0; j < src.GetComponentCount(); j++)
			{
				classes += " " + src.GetComponent(j).GetClassName();
			}

			PrintFormat("[AE3] item: %1 components:%2", prefab, classes);
			for (int i = 0, n = src.GetComponentCount(); i < n; i++)
			{
				IEntityComponentSource comp = src.GetComponent(i);
				string line = comp.GetClassName();
				string v;
				array<string> keys = {"MaxCumulativeVolume", "MaxItemSize", "UseCapacityCoefficient", "CapacityCoefficient", "Priority", "StoragePurpose"};
				foreach (string k : keys)
				{
					if (comp.Get(k, v))
						line += " " + k + "=" + v;
				}

				BaseContainer attrs = comp.GetObject("Attributes");
				if (attrs)
				{
					array<string> akeys = {"m_Size", "m_SlotType", "CommonItemType"};
					foreach (string k : akeys)
					{
						if (attrs.Get(k, v))
							line += " " + k + "=" + v;
					}

					BaseContainer phys = attrs.GetObject("ItemPhysAttributes");
					if (phys)
					{
						array<string> pkeys = {"Weight", "SizeSetupStrategy", "ItemDimensions", "ItemVolume", "DimensionScaler"};
						foreach (string k : pkeys)
						{
							if (phys.Get(k, v))
								line += " " + k + "=" + v;
						}
					}

					BaseContainerList custom = attrs.GetObjectArray("CustomAttributes");
					for (int c = 0; custom && c < custom.Count(); c++)
					{
						BaseContainer ca = custom.Get(c);
						if (ca.GetClassName() != "PreviewRenderAttributes")
							continue;

						array<string> rkeys = {"CameraPreset", "CameraOrbitAngles", "CameraDistanceToItem", "FOV", "ItemRenderRotation", "PreviewModel"};
						foreach (string k : rkeys)
						{
							if (ca.Get(k, v))
								line += " preview." + k + "=" + v;
						}
					}
				}

				if (line != comp.GetClassName())
					PrintFormat("[AE3] item: %1 %2", prefab, line);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void RunCommandline()
	{
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		string listPath;
		int ok, failed;

		if (GetArg("-ae3List", listPath))
		{
			foreach (string path : ReadList(listPath))
			{
				bool build = !path.EndsWith(".et") && !path.EndsWith(".layout") && !path.EndsWith(".conf") && !path.EndsWith(".ent");
				if (rm.RegisterResourceFile(path, build))
				{
					ok++;
					Print("[AE3] registered " + path);
				}
				else
				{
					failed++;
					Print("[AE3] FAILED " + path, LogLevel.WARNING);
				}
			}
		}

		if (GetArg("-ae3Resave", listPath))
		{
			foreach (string path : ReadList(listPath))
			{
				MetaFile meta = rm.GetMetaFile(path);
				if (!meta)
				{
					failed++;
					Print("[AE3] NO META " + path, LogLevel.WARNING);
					continue;
				}

				ResourceName resName = meta.GetResourceID();
				meta.Release();
				Resource res = BaseContainerTools.LoadContainer(resName);
				if (!res || !res.IsValid() || !BaseContainerTools.SaveContainer(res.GetResource().ToBaseContainer(), resName))
				{
					failed++;
					Print("[AE3] RESAVE FAILED " + resName, LogLevel.WARNING);
					continue;
				}

				ok++;
				Print("[AE3] resaved " + resName);
			}
		}

		if (GetArg("-ae3Rebuild", listPath))
		{
			foreach (string path : ReadList(listPath))
			{
				rm.RebuildResourceFile(path, "PC", false);
				ok++;
				Print("[AE3] rebuilt " + path);
			}
		}

		string inspect;
		if (GetArg("-ae3Inspect", inspect))
			InspectPlacing();

		if (GetArg("-ae3Items", listPath))
			InspectItems(listPath);

		PrintFormat("[AE3] done ok=%1 failed=%2", ok, failed);
		string keep;
		if (!GetArg("-ae3KeepOpen", keep))
			Workbench.Exit(0);
	}
}
