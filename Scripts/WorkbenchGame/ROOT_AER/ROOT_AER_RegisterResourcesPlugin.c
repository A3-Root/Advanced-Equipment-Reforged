//! Headless resource helper.
//! -aerList=<path|path...>: registers each listed absolute path.
//! -aerRebuild=<path|path...>: builds runtime resources (.xob from .fbx, .edds from .png).
//! -aerResave=<path|path...>: loads and re-saves each listed registered .et / .conf / .layout so Workbench writes its IDs.
//! -aerDump=<resource>><path>|...: writes each listed resource to the absolute path (reading vanilla configs).
//! -aerKeepOpen=1: stays open so asynchronous imports finish (the caller ends the process).
//! Command line: -wbmodule=ScriptEditor -plugin=ROOT_AER_RegisterResourcesPlugin -aerList=<path|path>
[WorkbenchPluginAttribute(name: "AER Register Resources", description: "Register or re-save listed files", wbModules: {"ResourceManager", "ScriptEditor"}, awesomeFontCode: 0xF0C7)]
class ROOT_AER_RegisterResourcesPlugin : WorkbenchPlugin
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

				PrintFormat("[AER] inspect: catalog US list %1 '%2' entities=%3", l, ident, cnt);
			}
		}

		Resource res = BaseContainerTools.LoadContainer("{59EF8ECAE1DCD417}Prefabs/Editor/Modes/EditorModeEdit.et");
		if (!res || !res.IsValid())
		{
			Print("[AER] inspect: edit mode prefab not loaded", LogLevel.WARNING);
			return;
		}

		IEntitySource src = res.GetResource().ToEntitySource();
		for (int i = 0, n = src.GetComponentCount(); i < n; i++)
		{
			IEntityComponentSource comp = src.GetComponent(i);
			if (comp.GetClassName() != "SCR_PlacingEditorComponent")
				continue;

			BaseContainerList regs = comp.GetObjectArray("m_Registries");
			PrintFormat("[AER] inspect: registries %1", regs.Count());
			for (int r = 0; r < regs.Count(); r++)
			{
				array<ResourceName> prefabs = {};
				regs.Get(r).Get("m_Prefabs", prefabs);
				string first;
				if (!prefabs.IsEmpty())
					first = prefabs[0];

				PrintFormat("[AER] inspect: registry %1 %2 prefabs=%3 first=%4", r, regs.Get(r).GetClassName(), prefabs.Count(), first);
				foreach (ResourceName prefab : prefabs)
				{
					if (!prefab.Contains("ROOT_AER"))
						continue;

					SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.ExtractEditableUIInfoFromPrefab(prefab);
					string name = "NULL";
					if (info)
						name = info.GetName();

					PrintFormat("[AER] inspect: %1 -> %2 (%3)", prefab, name, WidgetManager.Translate(name));
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
				Print("[AER] item: not loaded " + prefab, LogLevel.WARNING);
				continue;
			}

			IEntitySource src = res.GetResource().ToEntitySource();
			string classes;
			for (int j = 0; j < src.GetComponentCount(); j++)
			{
				classes += " " + src.GetComponent(j).GetClassName();
			}

			PrintFormat("[AER] item: %1 components:%2", prefab, classes);
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
					PrintFormat("[AER] item: %1 %2", prefab, line);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void RunCommandline()
	{
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		string listPath;
		int ok, failed;

		if (GetArg("-aerList", listPath))
		{
			foreach (string path : ReadList(listPath))
			{
				bool build = !path.EndsWith(".et") && !path.EndsWith(".layout") && !path.EndsWith(".conf") && !path.EndsWith(".ent");
				if (rm.RegisterResourceFile(path, build))
				{
					ok++;
					Print("[AER] registered " + path);
				}
				else
				{
					failed++;
					Print("[AER] FAILED " + path, LogLevel.WARNING);
				}
			}
		}

		if (GetArg("-aerResave", listPath))
		{
			foreach (string path : ReadList(listPath))
			{
				MetaFile meta = rm.GetMetaFile(path);
				if (!meta)
				{
					failed++;
					Print("[AER] NO META " + path, LogLevel.WARNING);
					continue;
				}

				ResourceName resName = meta.GetResourceID();
				meta.Release();
				Resource res = BaseContainerTools.LoadContainer(resName);
				if (!res || !res.IsValid() || !BaseContainerTools.SaveContainer(res.GetResource().ToBaseContainer(), resName))
				{
					failed++;
					Print("[AER] RESAVE FAILED " + resName, LogLevel.WARNING);
					continue;
				}

				ok++;
				Print("[AER] resaved " + resName);
			}
		}

		if (GetArg("-aerRebuild", listPath))
		{
			foreach (string path : ReadList(listPath))
			{
				rm.RebuildResourceFile(path, "PC", false);
				ok++;
				Print("[AER] rebuilt " + path);
			}
		}

		string inspect;
		if (GetArg("-aerInspect", inspect))
			InspectPlacing();

		if (GetArg("-aerItems", listPath))
			InspectItems(listPath);

		if (GetArg("-aerDump", listPath))
		{
			foreach (string pair : ReadList(listPath))
			{
				array<string> parts = {};
				pair.Split(">", parts, true);
				Resource dump = BaseContainerTools.LoadContainer(parts[0]);
				if (parts.Count() == 2 && dump && dump.IsValid() && BaseContainerTools.SaveContainer(dump.GetResource().ToBaseContainer(), parts[0], parts[1]))
				{
					ok++;
					Print("[AER] dumped " + parts[0]);
				}
				else
				{
					failed++;
					Print("[AER] DUMP FAILED " + pair, LogLevel.WARNING);
				}
			}
		}

		PrintFormat("[AER] done ok=%1 failed=%2", ok, failed);
		string keep;
		if (!GetArg("-aerKeepOpen", keep))
			Workbench.Exit(0);
	}
}
