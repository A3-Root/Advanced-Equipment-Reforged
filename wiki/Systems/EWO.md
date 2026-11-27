# EWO Packs (77th JSOC EWO mode)

Off by default (setting `m_bEwoMode`, AER System module, Game Master attribute "77th JSOC EWO mode"). Packs whose
prefab path contains a name in `m_sEwoPrefabs` (default `Radio_ANPRC77`, the AN/PRC-77 radio backpack) are EWO packs,
worn, carried or placed. Inventory "Use" on a pack in the own inventory or the vicinity opens the EWO panel; it updates
every 3 s and closes when the pack is out of reach, the player dies or EWO mode is turned off.

- Energy pool `m_fEwoEnergyMax` (400; 1 energy = 1 Wh = 1 % of a laptop battery), shown as a bar.
- Wireless network: start / stop, name (default `EWO Net N`) and password (random, 8 characters), gateway `77.95.N.1`
  (lowest free N 1-254), range `m_fEwoWifiRange` (100 m), drains `m_fEwoWifiDrain` (3 per minute) while broadcasting,
  stops when the pack is empty. External SSH into an EWO network is accepted only from EWO subnets. The network is a
  router that exists while it broadcasts, follows the pack and is not shown on the Map app.
- Laptops in the carrier's inventory with their charge; Charge / Stop. Each charging laptop gains
  `m_fEwoChargeRate` (10) percent per minute; charging ends when the laptop is used, leaves the inventory, is full or
  the pack is empty.
- Plug in / Unplug: the nearest running generator, battery or solar panel within `m_fEwoSourceRange` (10 m) recharges
  the pack at up to `m_fEwoRechargeRate` (5 per minute) from what its consumers leave; it unplugs when the source
  stops, is picked up or is out of range.

Server owned; checked every 3 s. A pack is tracked from its first panel request. Pack state is not saved by
persistence.
