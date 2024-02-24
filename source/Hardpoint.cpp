/* Hardpoint.cpp
Copyright (c) 2016 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Hardpoint.h"

#include "Audio.h"
#include "Body.h"
#include "Effect.h"
#include "Flotsam.h"
#include "Outfit.h"
#include "pi.h"
#include "Projectile.h"
#include "Random.h"
#include "Ship.h"
#include "Visual.h"

#include <cmath>
#include <map>

using namespace std;

namespace {
	// Create all the effects in the given list, at the given location, velocity, and angle.
	void CreateEffects(const map<const Effect *, int> &m, Point pos, Point vel, Angle angle, vector<Visual> &visuals)
	{
		for(const auto &it : m)
			for(int i = 0; i < it.second; ++i)
				visuals.emplace_back(*it.first, pos, vel, angle);
	}
}



// Constructor.
Hardpoint::Hardpoint(const Point &point, const Angle &baseAngle, bool isTurret,
	bool isParallel, bool isUnder, const Outfit *outfit)
	: outfit(outfit), point(point * .5), baseAngle(baseAngle), isTurret(isTurret), isParallel(isParallel), isUnder(isUnder)
{
	weapon = outfit ? &outfit->GetWeapon() : nullptr;
}



// Get the weapon in this hardpoint. This returns null if there is none.
const Outfit *Hardpoint::GetOutfit() const
{
	return outfit;
}



const Weapon *Hardpoint::GetWeapon() const
{
	return weapon;
}



// Get the location, relative to the center of the ship, from which
// projectiles of this weapon should originate.
const Point &Hardpoint::GetPoint() const
{
	return point;
}



// Get the convergence angle adjustment of this weapon (guns only, not turrets).
const Angle &Hardpoint::GetAngle() const
{
	return angle;
}



// Get the default facing direction for a gun
const Angle &Hardpoint::GetBaseAngle() const
{
	return baseAngle;
}



// Get the angle this weapon ought to point at for ideal gun harmonization.
Angle Hardpoint::HarmonizedAngle() const
{
	if(!outfit)
		return Angle();

	// Calculate reference point for non-forward facing guns.
	Angle rotateAngle = Angle() - baseAngle;
	Point refPoint = rotateAngle.Rotate(point);

	// Find the point of convergence of shots fired from this gun. That is,
	// find the angle where the projectile's X offset will be zero when it
	// reaches the very end of its range.
	double d = weapon->Range();
	// Projectiles with a range of zero should fire straight forward. A
	// special check is needed to avoid divide by zero errors.
	return Angle(d <= 0. ? 0. : -asin(refPoint.X() / d) * TO_DEG);
}



// Find out if this is a turret hardpoint (whether or not it has a turret installed).
bool Hardpoint::IsTurret() const
{
	return isTurret;
}




bool Hardpoint::IsParallel() const
{
	return isParallel;
}



bool Hardpoint::IsUnder() const
{
	return isUnder;
}



// Find out if this hardpoint has a homing weapon installed.
bool Hardpoint::IsHoming() const
{
	return weapon && weapon->Homing();
}



// Find out if this hardpoint has a special weapon installed
// (e.g. anti-missile, tractor beam).
bool Hardpoint::IsSpecial() const
{
	return weapon && (weapon->AntiMissile() || weapon->TractorBeam());
}



bool Hardpoint::CanAim() const
{
	return weapon && weapon->TurretTurn();
}



// Check if this weapon is ready to fire.
bool Hardpoint::IsReady() const
{
	return outfit && burstReload <= 0. && burstCount;
}



// Check if this weapon fired the last time it was able to fire. This is to
// figure out if the stream spacing timer should be applied or not.
bool Hardpoint::WasFiring() const
{
	return wasFiring;
}



// Get the number of remaining burst shots before a full reload is required.
int Hardpoint::BurstRemaining() const
{
	return burstCount;
}



// Perform one step (i.e. decrement the reload count).
void Hardpoint::Step()
{
	if(!outfit)
		return;

	wasFiring = isFiring;
	if(reload > 0.)
		--reload;
	// If the full reload time is elapsed, reset the burst counter.
	if(reload <= 0.)
		burstCount = weapon->BurstCount();
	if(burstReload > 0.)
		--burstReload;
	// If the burst reload time has elapsed, this weapon will not count as firing
	// continuously if it is not fired this frame.
	if(burstReload <= 0.)
		isFiring = false;
}



// Adjust this weapon's aim by the given amount, relative to its maximum
// "turret turn" rate.
void Hardpoint::Aim(double amount)
{
	if(!weapon)
		return;

	angle += weapon->TurretTurn() * amount;
}



// Fire this weapon. If it is a turret, it automatically points toward
// the given ship's target. If the weapon requires ammunition, it will
// be subtracted from the given ship.
void Hardpoint::Fire(Ship &ship, vector<Projectile> &projectiles, vector<Visual> &visuals)
{
	// Since this is only called internally by Armament (no one else has non-
	// const access), assume Armament checked that this is a valid call.
	Angle aim = ship.Facing();
	Point start = ship.Position() + aim.Rotate(point);

	// Apply the aim and hardpoint offset.
	aim += angle;
	start += aim.Rotate(weapon->HardpointOffset());

	// Apply the weapon's inaccuracy to the aim. This allows firing effects
	// to share the same inaccuracy as the projectile.
	aim += Distribution::GenerateInaccuracy(weapon->Inaccuracy(), weapon->InaccuracyDistribution());

	// Create a new projectile, originating from this hardpoint.
	// In order to get projectiles to start at the right position they are drawn
	// at an offset of (.5 * velocity). See BatchDrawList.cpp for more details.
	projectiles.emplace_back(ship, start - .5 * ship.Velocity(), aim, weapon);

	// Create any effects this weapon creates when it is fired.
	CreateEffects(weapon->FireEffects(), start, ship.Velocity(), aim, visuals);

	// Update the reload and burst counters, and expend ammunition if applicable.
	Fire(ship, start, aim);
}



// Fire an anti-missile. Returns true if the missile should be killed.
bool Hardpoint::FireAntiMissile(Ship &ship, const Projectile &projectile, vector<Visual> &visuals)
{
	// Make sure this hardpoint really is an anti-missile.
	int strength = weapon->AntiMissile();
	if(!strength)
		return false;

	// Check whether the projectile is within range and create any visuals.
	if(!FireSpecialSystem(ship, projectile, visuals))
		return false;

	// Check whether the missile was destroyed.
	return (Random::Int(strength) > Random::Int(projectile.MissileStrength()));
}



// Fire a tractor beam. Returns true if the flotsam was hit.
bool Hardpoint::FireTractorBeam(Ship &ship, const Flotsam &flotsam, std::vector<Visual> &visuals)
{
	// Make sure this hardpoint really is a tractor beam.
	double strength = weapon->TractorBeam();
	if(!strength)
		return false;

	// Check whether the flotsam is within range and create any visuals.
	if(!FireSpecialSystem(ship, flotsam, visuals))
		return false;

	return true;
}



// This weapon jammed. Increase its reload counters, but don't fire.
void Hardpoint::Jam()
{
	// Since this is only called internally by Armament (no one else has non-
	// const access), assume Armament checked that this is a valid call.

	// Reset the reload count.
	reload += weapon->Reload();
	burstReload += weapon->BurstReload();
}



// Install a weapon here (assuming it is empty). This is only for
// Armament to call internally.
void Hardpoint::Install(const Outfit *outfit)
{
	// If the given outfit is not a valid weapon, this hardpoint becomes empty.
	// Also check that the type of the weapon (gun or turret) is right.
	if(!outfit || !outfit->GetWeapon().IsWeapon() || (isTurret == !outfit->Get("turret mounts")))
		Uninstall();
	else
	{
		// Reset all the reload counters.
		this->outfit = outfit;
		weapon = &outfit->GetWeapon();
		Reload();

		// For fixed weapons, apply "gun harmonization," pointing them slightly
		// inward so the projectiles will converge. For turrets, start them out
		// pointing outward from the center of the ship.
		if(!isTurret)
		{
			angle = baseAngle;
			// Weapons that fire in parallel beams don't get a harmonized angle.
			// And some hardpoints/gunslots are configured not to get harmonized.
			// So only harmonize when both the port and the outfit supports it.
			if(!isParallel && !weapon->IsParallel())
				angle += HarmonizedAngle();
		}
		else
			angle = Angle(point);
	}
}



// Reload this weapon.
void Hardpoint::Reload()
{
	reload = 0.;
	burstReload = 0.;
	burstCount = weapon ? weapon->BurstCount() : 0;
}



// Uninstall the outfit from this port (if it has one).
void Hardpoint::Uninstall()
{
	outfit = nullptr;
	weapon = nullptr;
}



// Check whether a projectile or flotsam is within the range of the anti-missile
// or tractor beam system and create visuals if it is.
bool Hardpoint::FireSpecialSystem(Ship &ship, const Body &body, std::vector<Visual> &visuals)
{
	// Get the weapon range. Anti-missile and tractor beam shots always last a
	// single frame, so their range is equal to their velocity.
	double range = weapon->Velocity();

	// Check if the body is within range of this hardpoint.
	Point start = ship.Position() + ship.Facing().Rotate(point);
	Point offset = body.Position() - start;
	if(offset.Length() > range)
		return false;

	// Precompute the number of visuals that will be added.
	visuals.reserve(visuals.size() + weapon->FireEffects().size()
		+ weapon->HitEffects().size() + weapon->DieEffects().size());

	// Firing effects are displayed at the weapon hardpoint that just fired.
	Angle aim(offset);
	angle = aim - ship.Facing();
	start += aim.Rotate(weapon->HardpointOffset());
	CreateEffects(weapon->FireEffects(), start, ship.Velocity(), aim, visuals);

	// Figure out where the hit effect should be placed. Anti-missile and tractor
	// beam systems do not create projectiles; they just create a blast animation.
	CreateEffects(weapon->HitEffects(), start + (.5 * range) * aim.Unit(), ship.Velocity(), aim, visuals);

	// Die effects are displayed at the body, whether or not it actually "dies."
	CreateEffects(weapon->DieEffects(), body.Position(), body.Velocity(), aim, visuals);

	// Update the reload and burst counters, and expend ammunition if applicable.
	Fire(ship, start, aim);

	return true;
}



// Update any counters that change when this projectile fires.
void Hardpoint::Fire(Ship &ship, const Point &start, const Angle &aim)
{
	// Since this is only called internally, it is safe to assume that the
	// outfit pointer is not null.

	// Reset the reload count.
	reload += weapon->Reload();
	burstReload += weapon->BurstReload();
	--burstCount;
	isFiring = true;

	// Anti-missile sounds can be specified either in the outfit itself or in
	// the effect they create.
	if(weapon->WeaponSound())
		Audio::Play(weapon->WeaponSound(), start);
	// Apply any "kick" from firing this weapon.
	double force = weapon->FiringForce();
	if(force)
		ship.ApplyForce(aim.Unit() * -force);

	// Expend any ammo that this weapon uses. Do this as the very last thing, in
	// case the outfit is its own ammunition.
	ship.ExpendAmmo(*weapon);
}
