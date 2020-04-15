/* FormationPositioner.h
Copyright (c) 2019 by Peter van der Meer

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef FORMATION_POSITIONER_H_
#define FORMATION_POSITIONER_H_

#include "Body.h"
#include "FormationPattern.h"



// Represents an active formation for a set of spaceships. Assigns each ship
// to a position (Point) in the formation.
class FormationPositioner{
public:
	// Initializer based on the formation pattern to follow
	FormationPositioner(const Body * formationLead, const FormationPattern * pattern): formationLead(formationLead), pattern(pattern) {}
	
	// Start/reset/initialize for a (new) round of formation position calculations
	// for a formation around the ship given as parameter.
	void Start();
	
	// Get the point for the next ship in the formation. Caller should ensure
	// that the ships are offered in the right order to the calculator.
	Point NextPosition();
	
	
private:
	// The scaling factor currently being used.
	double activeScalingFactor = 80.;
	
	// Values used during ship position calculation iterations.
	int ring = 0;
	int activeLine = 0;
	int lineSlot = 0;
	int lineSlots = -1;
	
	// The body around which the formation will be formed and the pattern to follow.
	const Body * formationLead;
	const FormationPattern * pattern;
};



#endif
