# Answers "Register Resource File(s)" dialogs of headless Workbench runs (FBX "as Model", notices "OK") until the deadline.
# Usage: powershell -File tools/wb_autoanswer.ps1 [-Seconds 1800] [-ParentPid <pid to wait for>]
param([int]$Seconds = 1800, [int]$ParentPid = 0)
Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes
Add-Type @"
using System; using System.Text; using System.Runtime.InteropServices; using System.Collections.Generic;
public class AE3Win { public delegate bool P(IntPtr h, IntPtr l);
[DllImport("user32.dll")] public static extern bool EnumWindows(P p, IntPtr l);
[DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
[DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
public static List<IntPtr> Find(string title){ var r=new List<IntPtr>(); EnumWindows((h,l)=>{ if(IsWindowVisible(h)){var s=new StringBuilder(256); GetWindowText(h,s,256); if(s.ToString()==title) r.Add(h);} return true;},IntPtr.Zero); return r;} }
"@
$deadline = (Get-Date).AddSeconds($Seconds)
$ok = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::NameProperty, "OK")
$name = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::NameProperty, "as Model")
while ((Get-Date) -lt $deadline) {
    foreach ($h in [AE3Win]::Find("Register Resource File(s)")) {
        $w = [System.Windows.Automation.AutomationElement]::FromHandle($h)
        $b = $w.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $name)
        if (-not $b) { $b = $w.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $ok) }
        if ($b) { $b.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke() }
    }
    if ($ParentPid -ne 0 -and -not (Get-Process -Id $ParentPid -ErrorAction SilentlyContinue)) { break }
    Start-Sleep -Milliseconds 700
}
