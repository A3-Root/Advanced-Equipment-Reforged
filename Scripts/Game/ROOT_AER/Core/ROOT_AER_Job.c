//! Delayed action kept alive until it runs (simulated transfers, ping replies).
class ROOT_AER_Job : Managed
{
	protected static ref array<ref ROOT_AER_Job> s_aJobs = {};

	//------------------------------------------------------------------------------------------------
	//! Drops all state (game end).
	static void Reset()
	{
		foreach (ROOT_AER_Job job : s_aJobs)
		{
			if (job)
				GetGame().GetCallqueue().Remove(job.Fire);
		}

		s_aJobs.Clear();
	}

	//------------------------------------------------------------------------------------------------
	void Schedule(float delayMs)
	{
		s_aJobs.Insert(this);
		GetGame().GetCallqueue().CallLater(Fire, Math.Max(0, delayMs), false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Fire()
	{
		ROOT_AER_Job keep = this;
		s_aJobs.RemoveItem(this);
		keep.Run();
	}

	//------------------------------------------------------------------------------------------------
	void Run()
	{
	}
}
