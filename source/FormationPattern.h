/* FormationPattern.h
Copyright (c) 2019 by Peter van der Meer

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef FORMATION_PATTERN_H_
#define FORMATION_PATTERN_H_

#include "Angle.h"
#include "DataNode.h"
#include "Point.h"

#include <string>
#include <vector>



// Class that handles the loading and position calculations for a pattern that
// can be used for ships flying in formation.
// This class only deals with calculation the positions that exist in a formation
// pattern, the actual assignment of ships to positions is not handled in this class.
class FormationPattern {
public:
	// Returns the name of this pattern.
	const std::string Name() const;
	
	// Load formation from a datafile.
	void Load(const DataNode &node);
	
	// Retrieve properties like number of lines and arcs, number of repeat sections and number of positions.
	// TODO: Should we hide those properties and just provide a position iterator instead?
	unsigned int Lines() const;
	unsigned int Repeats(unsigned int lineNr) const;
	unsigned int Slots(unsigned int ring, unsigned int lineNr, unsigned int repeatNr) const;
	bool IsCentered(unsigned int lineNr) const;
	
	// Calculate a position based on the current ring, line/arc and slot on the line.
	Point Position(unsigned int ring, unsigned int lineNr, unsigned int repeatNr, unsigned int lineSlot, double diameterToPx, double widthToPx, double heightToPx) const;
	
	// Information about allowed rotating and mirroring that still results in the same formation.
	int Rotatable() const;
	bool FlippableY() const;
	bool FlippableX() const;
	
	
protected:
	// TODO: Should we make the classes here public or private?
	class MultiAxisPoint {
	public:
		// Coordinate axises for formations; Pixels (default) and heights, widths and diameters of the biggest ship in a formation.
		enum Axis { PIXELS, DIAMETERS, WIDTHS, HEIGHTS };
		
		// Add position information to one of the internal tracked points.
		void Add(Axis axis, const Point &toAdd);
		
		// Parse a position input from a data-node and add the values to this MultiAxisPoint.
		// This function is typically called when getting the first or last position on a
		// line or when getting an anchor for an arc.
		void AddLoad(const DataNode &node);
		
		// Get a point in pixel coordinates based on the conversion factors given for
		// the diameters, widths and heights.
		Point GetPx(double diameterToPx, double widthToPx, double heightToPx) const;
	
	
	private:
		// Position based on the possible axises.
		Point position[4];
	};
	
	class LineRepeat {
	public:
		// Vector to apply to get to the next start point for the next iteration.
		MultiAxisPoint repeatStart;
		MultiAxisPoint repeatEndOrAnchor;
		
		double repeatAngle = 0;

		// Slots to add or remove in this repeat section.
		int repeatSlots = 0;
		
		// Indicates if each odd repeat section should start from the end instead of the start.
		bool alternating = false;
	};

	class Line {
	public:
		// The starting point for this line.
		MultiAxisPoint start;
		MultiAxisPoint endOrAnchor;
		
		// Angle in case this line is an Arc.
		double angle = 0;
		
		// Sections of the line that repeat.
		std::vector<LineRepeat> repeats;
		
		// The number of initial positions for this line.
		int slots = 1;
		
		// Properties of how the line behaves
		bool centered = false;
		bool isArc = false;
		bool skipFirst = false;
		bool skipLast = false;
	};
	
	
protected:
	// The lines that define the formation.
	std::vector<Line> lines;
	
	
private:
	// Name of the formation pattern.
	std::string name;
	// Indicates if the formation is rotatable, a value of -1 means not
	// rotatable, while a positive value is taken as the rotation angle
	// in relation to the full 360 degrees full angle:
	// Square and Diamond shapes could get a value of 90, since you can
	// rotate such a shape over 90 degrees and still have the same shape.
	// Triangles could get a value of 120, since you can rotate them over
	// 120 degrees and again get the same shape.
	int rotatable = -1;
	// Indicates if the formation is flippable along the longitudinal axis.
	bool flippable_y = false;
	// Indicates if the formation is flippable along the transverse axis.
	bool flippable_x = false;
};



#endif
