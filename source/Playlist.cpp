/* Playlist.cpp
 Copyright (c) 2022 by RisingLeaf

 Endless Sky is free software: you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option) any later version.

 Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "DataNode.h"
#include "GameData.h"
#include "Playlist.h"
#include "PlayerInfo.h"
#include "System.h"


using namespace std;



Playlist::Playlist(const DataNode &node)
{
	Load(node);
}



void Playlist::Load(const DataNode &node)
{
	// All tracks need a name.
	if(node.Size() < 2)
	{
		node.PrintTrace("Error: No name specified for playlist:");
		return;
	}

	if(!name.empty())
	{
		node.PrintTrace("Error: Duplicate definition of playlist:");
		return;
	}
	name = node.Token(1);

	for(const DataNode &child : node)
	{
		if(child.Token(0) == "conditions")
			toPlay.Load(child);
		else if(child.Token(0) == "location" && child.Size() >= 2)
			location.Load(child);
		else if(child.Token(0) == "silence" && child.Size() >= 2)
		{
			silence = child.Value(1);
			if(child.Size() >= 3)
				silenceLimit = child.Value(2);
			if(child.Size() >= 4)
				silenceProb = child.Value(3);
		}
		else if(child.Token(0) == "priority" && child.Size() >= 2)
			priority = child.Value(1);
		else if(child.Token(0) == "weight" && child.Size() >= 2)
			weight = child.Value(1);
		else if(child.Token(0) == "override")
			overridePlaylist = true;
		else if(child.Token(0) == "tracks")
		{
			if(child.Size() >= 2)
				progressionStyle = child.Token(1);
			else
				progressionStyle = "linear";
			for(const auto &grand : child)
				if(grand.Size() >= 2)
					tracks.emplace_back(grand.Value(1), GameData::Tracks().Get(grand.Token(0)));
				else
					tracks.emplace_back(10, GameData::Tracks().Get(grand.Token(0)));
		}
	}
}



const Track *Playlist::GetRandomTrack() const
{
	return tracks.Get();
}



bool Playlist::MatchingConditions(PlayerInfo &player) const
{
	return toPlay.Test(player.Conditions()) && location.Matches(player.GetSystem());
}



int Playlist::Priority() const
{
	return priority;
}



int Playlist::Weight() const
{
	return weight;
}
