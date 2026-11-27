# Power

| Role | Behaviour |
| --- | --- |
| Consumer | Draws on / standby watts from one provider; runs on its internal battery when unsupplied; a consumer with a fuel tank (floodlight) runs on the tank |
| Battery | Charges from the output its provider has left after consumers, supplies its own consumers, switches off on overload |
| Generator | Burns fuel while on, switches off on overload or an empty tank; refuel from a carried fuel container |
| Solar panel | Output by sun elevation and panel tilt, buffered in its internal battery |

Internal batteries and solar buffers deliver up to `m_fBufferOutputW` (default 1000 W). Devices with "Runs only on
external power" use no internal battery. Mains-powered devices (portable server) are always supplied. The simulation
runs on the server every `m_fPowerUpdateInterval` (5 s); battery levels replicate when they change by
`m_fPowerChangeThreshold`.

## Fuel

Generators and consumers with a tank (`m_fFuelCapacityL`) burn `m_fBurnLph` litres per hour while on. Fuel use 0 runs
without using fuel. Game Master attributes "Fuel" (percent of the tank) and "Fuel use"; script `SetFuel`, `SetFuelUse`.
Refuel drains a carried fuel container.

## Links

A consumer or battery has one provider. Links are made by:

- players: "Take power cable" on the source, "Plug power cable into ..." on the device (see
  [Cables and Flash Drives](Cables-and-Flash-Drives.md));
- the Game Master: "Power source" (sources within 500 m), "Connect power (selection)";
- the World Editor: `ROOT_AE3_ConnectionComponent.m_sPowerSource`;
- script: `ROOT_AE3_API.ConnectPower(consumer, source)`.

A link drops when either device is picked up or they move farther apart than the cable. The power cable length of a
source is `ROOT_AE3_PowerComponent.m_fCableLength` (Game Master "Power cable length", `SetPowerCableLength`).

## States

Off, on, standby (computers), crashed (blue screen; needs a power cycle). Game Master: Power on / off, Standby, Crash.
Script: `SetPowerState(device, 0 off | 1 on | 2 standby)`, `CrashDevice`, `GetPowerState`.
