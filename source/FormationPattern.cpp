/* FormationPattern.cpp
Copyright (c) 2019 by Peter van der Meer

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "FormationPattern.h"

#include "Angle.h"
#include "DataNode.h"
#include "Point.h"

#include <vector>

using namespace std;



const string FormationPattern::Name() const
{
	return name;
}



FormationPattern::Line::Line(double X, double Y, int slots, double directionAngle)
{
	anchor = Point(X, Y);
	initialSlots = slots;
	direction = Angle(directionAngle);
};



void FormationPattern::Load(const DataNode &node)
{
	if(node.Size() >=2)
		name = node.Token(1);
	
	for(const DataNode &child : node)
	{
		if(child.Size() >= 5 && child.Token(0) == "line")
		{
			lines.emplace_back(child.Value(1), child.Value(2), static_cast<int>(child.Value(3) + 0.5), child.Value(4));
			Line &line = lines[lines.size()-1];
			for(const DataNode &grand : child)
			{
				if(grand.Size() >= 2 && grand.Token(0) == "spacing")
					line.spacing = grand.Value(1);
				else if(grand.Size() >= 3 && grand.Token(0) == "repeat")
				{
					line.repeatVector = Point(grand.Value(1), grand.Value(2));
					line.slotsIncrease = 0;
					for(const DataNode &grandGrand : grand)
						if(grandGrand.Size() >= 2 && grandGrand.Token(0) == "increase")
							line.slotsIncrease = static_cast<int>(grandGrand.Value(1) + 0.5);
				}
			}
		}
	}
}



// Get the next line that has space for placement of ships. Returns -1
// if none found/available.
int FormationPattern::NextLine(unsigned int iteration, unsigned int lineNr) const
{
	// All lines participate in the first iteration
	if(iteration == 0 && lineNr < (lines.size()-1))
		return lineNr + 1;
	
	// For later iterations only lines that repeat will participate
	unsigned int linesScanned = 0;
	while(linesScanned <= lines.size())
	{
		lineNr = (lineNr + 1) % lines.size();
		if((lines[lineNr]).slotsIncrease >= 0)
			return lineNr;
		
		// Safety mechanism to avoid endless loops if the formation has a limited size.
		linesScanned++;
	}
	return -1;
}



// Get the number of positions on a line for the given iteration
int FormationPattern::PositionsOnLine(unsigned int iteration, unsigned int lineNr) const
{
	if(lineNr >= lines.size())
		return 0;
	
	// Retrieve the relevant line
	Line line = lines[lineNr];
	
	// For the first iteration, only the initial positions are relevant
	if(iteration == 0)
		return line.initialSlots;
	
	// If we are in a later iteration, then skip lines that don't repeat
	if(line.slotsIncrease < 0)
		return 0;
	
	return line.initialSlots + line.slotsIncrease * iteration;
}



// Get a formation position based on iteration, line-number and position on the line.
Point FormationPattern::Position(unsigned int iteration, unsigned int lineNr, unsigned int posOnLine) const
{
	if(lineNr >= lines.size())
		return Point();
	
	Line line = lines[lineNr];
	
	// Calculate position based
	return line.anchor +
		line.repeatVector * iteration +
		line.direction.Rotate(Point(0, -line.spacing * posOnLine));
}
