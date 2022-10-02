/* PrintData.cpp
Copyright (c) 2014 by Michael Zahniser, 2022 by warp-core

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "PrintData.h"

#include "GameData.h"
#include "Outfit.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"

#include <iostream>
#include <map>
#include <set>

using namespace std;



bool PrintData::IsPrintDataArgument(const char *const *argv)
{
	for(const char *const *it = argv + 1; *it; ++it)
	{
		string arg = *it;
		if(arg == "-s" || arg == "--ships" || arg == "-w" || arg == "--weapons" ||
				arg == "-o" || arg == "--outfits" || arg == "-e" || arg == "--engines" ||
				arg == "--power" || arg == "--planets" || arg == "--systems")
			return true;
	}
	return false;
}



void PrintData::Print(const char *const *argv)
{
	for(const char *const *it = argv + 1; *it; ++it)
	{
		string arg = *it;
		if(arg == "-s" || arg == "--ships")
			Ships(argv);
		else if(arg == "-w" || arg == "--weapons")
			PrintWeaponStats();
		else if(arg == "-e" || arg == "--engines")
			PrintEngineStats();
		else if(arg == "--power")
			PrintPowerStats();
		else if(arg == "-o" || arg == "--outfits")
			Outfits(argv);
		else if(arg == "--planets")
			Planets(argv);
		else if(arg == "--systems")
			Systems(argv);
	}
	cout.flush();
}



void PrintData::Help()
{
	cerr << "    -s, --ships: prints a table of ship stats (just the base stats, not considering any stored outfits)."
			<< endl;
	cerr << "        --sales: prints a table of ships with every 'shipyard' each appears in." << endl;
	cerr << "        --loaded: prints a table of ship stats accounting for installed outfits. Does not include variants."
			<< endl;
	cerr << "        --list: prints a list of all ship names." << endl;
	cerr << "    Use the modifier `--variants` with the above two commands to include variants." << endl;
	cerr << "    -w, --weapons: prints a table of weapon stats." << endl;
	cerr << "    -e, --engines: prints a table of engine stats." << endl;
	cerr << "    --power: prints a table of power outfit stats." << endl;
	cerr << "    -o, --outfits: prints a list of outfits." << endl;
	cerr << "        --sales: prints a list of outfits and every 'outfitter' each appears in." << endl;
	cerr << "        -a, --all: prints a table of outfits and all attributes used by any outfits present." << endl;
	cerr << "    --planets: prints a list of all planets." << endl;
	cerr << "        --descriptions: prints a table of all planets and their descriptions." << endl;
	cerr << "        --attributes: prints a table of all planets and their attributes." << endl;
	cerr << "            --reverse: prints a table of all planet attributes and which planets have them."
			<< endl;
	cerr << "    --systems: prints a list of all systems." << endl;
	cerr << "        --attributes: prints a list of all systems and their attributes." << endl;
	cerr << "            --reverse: prints a list of all system attributes and which systems have them."
			<< endl;
}



void PrintData::Ships(const char *const *argv)
{
	bool loaded = false;
	bool variants = false;
	bool sales = false;
	bool list = false;

	for(const char *const *it = argv + 2; *it; ++it)
	{
		string arg = *it;
		if(arg == "--variants")
			variants = true;
		else if(arg == "--sales")
			sales = true;
		else if(arg == "--loaded")
			loaded = true;
		else if(arg == "--list")
			list = true;
	}

	if(sales)
		PrintShipShipyards();
	else if(loaded)
		PrintLoadedShipStats(variants);
	else if(list)
		PrintShipList(variants);
	else
		PrintBaseShipStats();
}



void PrintData::PrintBaseShipStats()
{
	cout << "model" << ',' << "category" << ',' << "chassis cost" << ',' << "loaded cost" << ',' << "shields" << ','
		<< "hull" << ',' << "mass" << ',' << "drag" << ',' << "heat dissipation" << ','
		<< "required crew" << ',' << "bunks" << ',' << "cargo space" << ',' << "fuel" << ','
		<< "outfit space" << ',' << "weapon capacity" << ',' << "engine capacity" << ',' << "gun mounts" << ','
		<< "turret mounts" << ',' << "fighter bays" << ',' << "drone bays" << '\n';
	for(auto &it : GameData::Ships())
	{
		// Skip variants and unnamed / partially-defined ships.
		if(it.second.ModelName() != it.first)
			continue;

		const Ship &ship = it.second;
		cout << it.first << ',';

		const Outfit &attributes = ship.BaseAttributes();
		cout << attributes.Category() << ',';
		cout << ship.ChassisCost() << ',';
		cout << ship.Cost() << ',';

		auto mass = attributes.Mass() ? attributes.Mass() : 1.;
		cout << attributes.Get("shields") << ',';
		cout << attributes.Get("hull") << ',';
		cout << mass << ',';
		cout << attributes.Get("drag") << ',';
		cout << ship.HeatDissipation() * 1000. << ',';
		cout << attributes.Get("required crew") << ',';
		cout << attributes.Get("bunks") << ',';
		cout << attributes.Get("cargo space") << ',';
		cout << attributes.Get("fuel capacity") << ',';

		cout << attributes.Get("outfit space") << ',';
		cout << attributes.Get("weapon capacity") << ',';
		cout << attributes.Get("engine capacity") << ',';

		int numTurrets = 0;
		int numGuns = 0;
		for(auto &hardpoint : ship.Weapons())
		{
			if(hardpoint.IsTurret())
				++numTurrets;
			else
				++numGuns;
		}
		cout << numGuns << ',' << numTurrets << ',';

		int numFighters = ship.BaysTotal("Fighter");
		int numDrones = ship.BaysTotal("Drone");
		cout << numFighters << ',' << numDrones << '\n';
	}
}



void PrintData::PrintShipShipyards()
{
	cout << "ship" << ',' << "shipyards" << '\n';
	map<string, set<string>> ships;
	for(auto &it : GameData::Shipyards())
	{
		for(auto &it2 : it.second)
		{
			ships[it2->ModelName()].insert(it.first);
		}
	}
	for(auto &it : GameData::Ships())
	{
		if(it.first != it.second.ModelName())
			continue;
		cout << it.first;
		for(auto &it2 : ships[it.first])
		{
			cout << ',' << it2;
		}
		cout << '\n';
	}
}



void PrintData::PrintLoadedShipStats(bool variants)
{
	cout << "model" << ',' << "category" << ',' << "cost" << ',' << "shields" << ','
		<< "hull" << ',' << "mass" << ',' << "required crew" << ',' << "bunks" << ','
		<< "cargo space" << ',' << "fuel" << ',' << "outfit space" << ',' << "weapon capacity" << ','
		<< "engine capacity" << ',' << "speed" << ',' << "accel" << ',' << "turn" << ','
		<< "energy generation" << ',' << "max energy usage" << ',' << "energy capacity" << ','
		<< "idle/max heat" << ',' << "max heat generation" << ',' << "max heat dissipation" << ','
		<< "gun mounts" << ',' << "turret mounts" << ',' << "fighter bays" << ','
		<< "drone bays" << ',' << "deterrence" << '\n';
	for(auto &it : GameData::Ships())
	{
		// Skip variants and unnamed / partially-defined ships, unless specified otherwise.
		if(it.second.ModelName() != it.first && !variants)
			continue;

		const Ship &ship = it.second;
		cout << it.first << ',';

		const Outfit &attributes = ship.Attributes();
		cout << attributes.Category() << ',';
		cout << ship.Cost() << ',';

		auto mass = attributes.Mass() ? attributes.Mass() : 1.;
		cout << attributes.Get("shields") << ',';
		cout << attributes.Get("hull") << ',';
		cout << mass << ',';
		cout << attributes.Get("required crew") << ',';
		cout << attributes.Get("bunks") << ',';
		cout << attributes.Get("cargo space") << ',';
		cout << attributes.Get("fuel capacity") << ',';

		cout << ship.BaseAttributes().Get("outfit space") << ',';
		cout << ship.BaseAttributes().Get("weapon capacity") << ',';
		cout << ship.BaseAttributes().Get("engine capacity") << ',';
		cout << (attributes.Get("drag") ? (60. * attributes.Get("thrust") / attributes.Get("drag")) : 0) << ',';
		cout << 3600. * attributes.Get("thrust") / mass << ',';
		cout << 60. * attributes.Get("turn") / mass << ',';

		double energyConsumed = attributes.Get("energy consumption")
			+ max(attributes.Get("thrusting energy"), attributes.Get("reverse thrusting energy"))
			+ attributes.Get("turning energy")
			+ attributes.Get("afterburner energy")
			+ attributes.Get("fuel energy")
			+ (attributes.Get("hull energy") * (1 + attributes.Get("hull energy multiplier")))
			+ (attributes.Get("shield energy") * (1 + attributes.Get("shield energy multiplier")))
			+ attributes.Get("cooling energy")
			+ attributes.Get("cloaking energy");

		double heatProduced = attributes.Get("heat generation") - attributes.Get("cooling")
			+ max(attributes.Get("thrusting heat"), attributes.Get("reverse thrusting heat"))
			+ attributes.Get("turning heat")
			+ attributes.Get("afterburner heat")
			+ attributes.Get("fuel heat")
			+ (attributes.Get("hull heat") * (1. + attributes.Get("hull heat multiplier")))
			+ (attributes.Get("shield heat") * (1. + attributes.Get("shield heat multiplier")))
			+ attributes.Get("solar heat")
			+ attributes.Get("cloaking heat");

		for(const auto &oit : ship.Outfits())
			if(oit.first->IsWeapon() && oit.first->Reload())
			{
				double reload = oit.first->Reload();
				energyConsumed += oit.second * oit.first->FiringEnergy() / reload;
				heatProduced += oit.second * oit.first->FiringHeat() / reload;
			}
		cout << 60. * (attributes.Get("energy generation") + attributes.Get("solar collection")) << ',';
		cout << 60. * energyConsumed << ',';
		cout << attributes.Get("energy capacity") << ',';
		cout << ship.IdleHeat() / max(1., ship.MaximumHeat()) << ',';
		cout << 60. * heatProduced << ',';
		// Maximum heat is 100 degrees per ton. Bleed off rate is 1/1000 per 60th of a second, so:
		cout << 60. * ship.HeatDissipation() * ship.MaximumHeat() << ',';

		int numTurrets = 0;
		int numGuns = 0;
		for(auto &hardpoint : ship.Weapons())
		{
			if(hardpoint.IsTurret())
				++numTurrets;
			else
				++numGuns;
		}
		cout << numGuns << ',' << numTurrets << ',';

		int numFighters = ship.BaysTotal("Fighter");
		int numDrones = ship.BaysTotal("Drone");
		cout << numFighters << ',' << numDrones << ',';

		double deterrence = 0.;
		for(const Hardpoint &hardpoint : ship.Weapons())
			if(hardpoint.GetOutfit())
			{
				const Outfit *weapon = hardpoint.GetOutfit();
				if(weapon->Ammo() && !ship.OutfitCount(weapon->Ammo()))
					continue;
				double damage = weapon->ShieldDamage() + weapon->HullDamage()
					+ (weapon->RelativeShieldDamage() * ship.Attributes().Get("shields"))
					+ (weapon->RelativeHullDamage() * ship.Attributes().Get("hull"));
				deterrence += .12 * damage / weapon->Reload();
			}
		cout << deterrence << '\n';
	}
}



void PrintData::PrintShipList(bool variants)
{
	for(auto &it : GameData::Ships())
	{
		// Skip variants and unnamed / partially-defined ships, unless specified otherwise.
		if(it.second.ModelName() != it.first && !variants)
			continue;

		cout << "\"" << it.first << "\"\n";
	}
}



void PrintData::PrintWeaponStats()
{
	cout << "name" << ',' << "category" << ',' << "cost" << ',' << "space" << ',' << "range" << ','
		<< "reload" << ',' << "burst count" << ',' << "burst reload" << ',' << "lifetime" << ','
		<< "shots/second" << ',' << "energy/shot" << ',' << "heat/shot" << ',' << "recoil/shot" << ','
		<< "energy/s" << ',' << "heat/s" << ',' << "recoil/s" << ',' << "shield/s" << ','
		<< "discharge/s" << ',' << "hull/s" << ',' << "corrosion/s" << ',' << "heat dmg/s" << ','
		<< "burn dmg/s" << ',' << "energy dmg/s" << ',' << "ion dmg/s" << ',' << "slow dmg/s" << ','
		<< "disruption dmg/s" << ',' << "piercing" << ',' << "fuel dmg/s" << ',' << "leak dmg/s" << ','
		<< "push/s" << ',' << "homing" << ',' << "strength" << ',' << "deterrence" << '\n';

	for(auto &it : GameData::Outfits())
	{
		// Skip non-weapons and submunitions.
		if(!it.second.IsWeapon() || it.second.Category().empty())
			continue;

		const Outfit &outfit = it.second;
		cout << it.first << ',';
		cout << outfit.Category() << ',';
		cout << outfit.Cost() << ',';
		cout << -outfit.Get("weapon capacity") << ',';

		cout << outfit.Range() << ',';

		double reload = outfit.Reload();
		cout << reload << ',';
		cout << outfit.BurstCount() << ',';
		cout << outfit.BurstReload() << ',';
		cout << outfit.TotalLifetime() << ',';
		double fireRate = 60. / reload;
		cout << fireRate << ',';

		double firingEnergy = outfit.FiringEnergy();
		cout << firingEnergy << ',';
		firingEnergy *= fireRate;
		double firingHeat = outfit.FiringHeat();
		cout << firingHeat << ',';
		firingHeat *= fireRate;
		double firingForce = outfit.FiringForce();
		cout << firingForce << ',';
		firingForce *= fireRate;

		cout << firingEnergy << ',';
		cout << firingHeat << ',';
		cout << firingForce << ',';

		double shieldDmg = outfit.ShieldDamage() * fireRate;
		cout << shieldDmg << ',';
		double dischargeDmg = outfit.DischargeDamage() * 100. * fireRate;
		cout << dischargeDmg << ',';
		double hullDmg = outfit.HullDamage() * fireRate;
		cout << hullDmg << ',';
		double corrosionDmg = outfit.CorrosionDamage() * 100. * fireRate;
		cout << corrosionDmg << ',';
		double heatDmg = outfit.HeatDamage() * fireRate;
		cout << heatDmg << ',';
		double burnDmg = outfit.BurnDamage() * 100. * fireRate;
		cout << burnDmg << ',';
		double energyDmg = outfit.EnergyDamage() * fireRate;
		cout << energyDmg << ',';
		double ionDmg = outfit.IonDamage() * 100. * fireRate;
		cout << ionDmg << ',';
		double slowDmg = outfit.SlowingDamage() * fireRate;
		cout << slowDmg << ',';
		double disruptDmg = outfit.DisruptionDamage() * fireRate;
		cout << disruptDmg << ',';
		cout << outfit.Piercing() << ',';
		double fuelDmg = outfit.FuelDamage() * fireRate;
		cout << fuelDmg << ',';
		double leakDmg = outfit.LeakDamage() * 100. * fireRate;
		cout << leakDmg << ',';
		double hitforce = outfit.HitForce() * fireRate;
		cout << hitforce << ',';

		cout << outfit.Homing() << ',';
		double strength = outfit.MissileStrength() + outfit.AntiMissile();
		cout << strength << ',';

		double damage = outfit.ShieldDamage() + outfit.HullDamage();
		double deterrence = .12 * damage / outfit.Reload();
		cout << deterrence << '\n';
	}
	cout.flush();
}



void PrintData::PrintEngineStats()
{
	cout << "name" << '\t' << "cost" << '\t' << "mass" << '\t' << "outfit space" << '\t'
		<< "engine capacity" << '\t' << "thrust/s" << '\t' << "thrust energy/s" << '\t'
		<< "thrust heat/s" << '\t' << "turn/s" << '\t' << "turn energy/s" << '\t'
		<< "turn heat/s" << '\t' << "reverse thrust/s" << '\t' << "reverse energy/s" << '\t'
		<< "reverse heat/s" << '\n';
	for(auto &it : GameData::Outfits())
	{
		// Skip non-engines.
		if(it.second.Category() != "Engines")
			continue;

		const Outfit &outfit = it.second;
		cout << it.first << ',';
		cout << outfit.Cost() << ',';
		cout << outfit.Mass() << ',';
		cout << outfit.Get("outfit space") << ',';
		cout << outfit.Get("engine capacity") << ',';
		cout << outfit.Get("thrust") * 3600. << ',';
		cout << outfit.Get("thrusting energy") * 60. << ',';
		cout << outfit.Get("thrusting heat") * 60. << ',';
		cout << outfit.Get("turn") * 60. << ',';
		cout << outfit.Get("turning energy") * 60. << ',';
		cout << outfit.Get("turning heat") * 60. << ',';
		cout << outfit.Get("reverse thrust") * 3600. << ',';
		cout << outfit.Get("reverse thrusting energy") * 60. << ',';
		cout << outfit.Get("reverse thrusting heat") * 60. << '\n';
	}
	cout.flush();
}



void PrintData::PrintPowerStats()
{
	cout << "name" << ',' << "cost" << ',' << "mass" << ',' << "outfit space" << ','
		<< "energy generation" << ',' << "heat generation" << ',' << "energy capacity" << '\n';
	for(auto &it : GameData::Outfits())
	{
		// Skip non-power.
		if(it.second.Category() != "Power")
			continue;

		const Outfit &outfit = it.second;
		cout << it.first << ',';
		cout << outfit.Cost() << ',';
		cout << outfit.Mass() << ',';
		cout << outfit.Get("outfit space") << ',';
		cout << outfit.Get("energy generation") << ',';
		cout << outfit.Get("heat generation") << ',';
		cout << outfit.Get("energy capacity") << '\n';
	}
	cout.flush();
}



void PrintData::Outfits(const char *const *argv)
{
	bool sales = false;
	bool all = false;

	for(const char *const *it = argv + 2; *it; ++it)
	{
		string arg = *it;
		if(arg == "-s" || arg == "--sales")
			sales = true;
		else if(arg == "-a" || arg == "--all")
			all = true;
	}

	if(sales)
		PrintOutfitOutfitters();
	else if(all)
		PrintOutfitsAllStats();
	else
		PrintOutfitsList();
}



void PrintData::PrintOutfitsList()
{
	for(auto &it : GameData::Outfits())
		cout << "\"" << it.first << "\"\n";
}



void PrintData::PrintOutfitOutfitters()
{
	cout << "outfits" << ',' << "outfitters" << '\n';
	map<string, set<string>> outfits;
	for(auto &it : GameData::Outfitters())
	{
		for(auto &it2 : it.second)
		{
			outfits[it2->Name()].insert(it.first);
		}
	}
	for(auto &it : GameData::Outfits())
	{
		cout << it.first;
		for(auto &it2 : outfits[it.first])
		{
			cout << ',' << it2;
		}
		cout << '\n';
	}
}



void PrintData::PrintOutfitsAllStats()
{
	set<string> attributes;
	for(auto &it : GameData::Outfits())
	{
		const Outfit &outfit = it.second;
		for(const auto &attribute : outfit.Attributes())
			attributes.insert(attribute.first);
	}
	cout << "name" << ',' << "category" << ',' << "cost" << ',' << "mass";
	for(const auto &attribute : attributes)
		cout << ',' << attribute;
	cout << '\n';
	for(auto &it : GameData::Outfits())
	{
		const Outfit &outfit = it.second;
		cout << outfit.Name() << ',' << outfit.Category() << ','
			<< outfit.Cost() << ',' << outfit.Mass();
		for(const auto &attribute : attributes)
			cout << ',' << outfit.Attributes().Get(attribute);
		cout << '\n';
	}
}



void PrintData::Planets(const char *const *argv)
{
	bool descriptions = false;
	bool attributes = false;
	bool byAttribute = false;

	for(const char *const *it = argv + 2; *it; ++it)
	{
		string arg = *it;
		if(arg == "--descriptions")
			descriptions = true;
		else if(arg == "--attributes")
			attributes = true;
		else if(arg == "--reverse")
			byAttribute = true;
	}
	if(descriptions)
		PrintPlanetDescriptions();
	if(attributes && byAttribute)
		PrintPlanetsByAttribute();
	else if(attributes)
		PrintPlanetAttributes();
	if(!(descriptions || attributes))
		PrintPlanetsList();
}



void PrintData::PrintPlanetsList()
{
	cout << "planet" << '\n';
	for(auto &it : GameData::Planets())
		cout << it.first << '\n';
}



void PrintData::PrintPlanetDescriptions()
{
	cout << "planet::description::spaceport\n";
	for(auto &it : GameData::Planets())
	{
		cout << it.first << "::";
		const Planet &planet = it.second;
		cout << planet.Description() << "::";
		cout << planet.SpaceportDescription() << "\n";
	}
}



void PrintData::PrintPlanetAttributes()
{
	cout << "planet" << ',' << "attributes" << '\n';
	for(auto &it : GameData::Planets())
	{
		cout << it.first;
		const Planet &planet = it.second;
		for(const string &attribute : planet.Attributes())
			cout << ',' << attribute;
		cout << '\n';
	}
}



void PrintData::PrintPlanetsByAttribute()
{
	cout << "attribute" << ',' << "planets" << '\n';
	set<string> attributes;
	for(auto &it : GameData::Planets())
	{
		const Planet &planet = it.second;
		for(const string &attribute : planet.Attributes())
			attributes.insert(attribute);
	}
	for(const string &attribute : attributes)
	{
		cout << attribute;
		for(auto &it : GameData::Planets())
		{
			const Planet &planet = it.second;
			if(planet.Attributes().count(attribute))
				cout << ',' << it.first;
		}
		cout << '\n';
	}
}



void PrintData::Systems(const char *const *argv)
{
	bool attributes = false;
	bool byAttribute = false;

	for(const char *const *it = argv + 2; *it; ++it)
	{
		string arg = *it;
		if(arg == "--attributes")
			attributes = true;
		else if(arg == "--reverse")
			byAttribute = true;
	}
	if(attributes && byAttribute)
		PrintSystemsByAttribute();
	else if(attributes)
		PrintSystemAttributes();
	else
		PrintSystemsList();
}



void PrintData::PrintSystemsList()
{
	cout << "system" << '\n';
	for(auto &it : GameData::Systems())
		cout << it.first << '\n';
}



void PrintData::PrintSystemAttributes()
{
	cout << "system" << ',' << "attributes" << '\n';
	for(auto &it : GameData::Systems())
	{
		cout << it.first;
		const System &system = it.second;
		for(const string &attribute : system.Attributes())
			cout << ',' << attribute;
		cout << '\n';
	}
}



void PrintData::PrintSystemsByAttribute()
{
	cout << "attribute" << ',' << "systems" << '\n';
	set<string> attributes;
	for(auto &it : GameData::Systems())
	{
		const System &system = it.second;
		for(const string &attribute : system.Attributes())
			attributes.insert(attribute);
	}
	for(const string &attribute : attributes)
	{
		cout << attribute;
		for(auto &it : GameData::Systems())
		{
			const System &system = it.second;
			if(system.Attributes().count(attribute))
				cout << ',' << it.first;
		}
		cout << '\n';
	}
}



