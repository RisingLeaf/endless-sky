/* Plugins.cpp
Copyright (c) 2022 by Sam Gleske (samrocketman on GitHub)

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Plugins.h"

#include "DataFile.h"
#include "DataNode.h"
#include "DataWriter.h"
#include "Files.h"
#include "Logger.h"
#include "PluginHelper.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <map>
#include <string>

using namespace std;

namespace {
	Set<Plugin> plugins;

	void LoadSettingsFromFile(const string &path)
	{
		DataFile prefs(path);
		for(const DataNode &node : prefs)
		{
			if(node.Token(0) != "state")
				continue;

			for(const DataNode &child : node)
				if(child.Size() == 2)
				{
					auto *plugin = plugins.Get(child.Token(0));
					plugin->enabled = child.Value(1);
					plugin->currentState = child.Value(1);
				}
		}
	}

	bool oldNetworkActivity = false;
	atomic<int> currentBackgroundActivity{0};
}



// Checks whether this plugin is valid, i.e. whether it exists.
bool Plugin::IsValid() const
{
	return !name.empty();
}



// Try to load a plugin at the given path. Returns true if loading succeeded.
const Plugin *Plugins::Load(const string &path)
{
	// Get the name of the folder containing the plugin.
	size_t pos = path.rfind('/', path.length() - 2) + 1;
	string name = path.substr(pos, path.length() - 1 - pos);

	auto *plugin = plugins.Get(name);

	string pluginFile = path + "plugin.txt";

	// Load plugin metadata from plugin.txt.
	DataFile file(pluginFile);
	for(const DataNode &child : file)
	{
		if(child.Token(0) == "name" && child.Size() >= 2)
			plugin->name = child.Token(1);
		else if(child.Token(0) == "about" && child.Size() >= 2)
			plugin->aboutText = child.Token(1);
	}

	// Set missing required values.
	if(plugin->name.empty())
	{
		plugin->name = std::move(name);
		if(Files::Exists(pluginFile))
		{
			Logger::LogError(
				"Failed to find name field in plugin.txt. Defaulting plugin name to folder name: \"" + plugin->name + "\"");
		}
	}
	// Set values from old about.txt files.
	if(plugin->aboutText.empty())
		plugin->aboutText = Files::Read(path + "about.txt");

	plugin->path = path;

	plugin->aboutText = Files::Read(path + "about.txt");
	plugin->version = Files::Read(path + "version.txt");

	return plugin;
}



void Plugins::LoadSettings()
{
	// Global plugin settings
	LoadSettingsFromFile(Files::Resources() + "plugins.txt");
	// Local plugin settings
	LoadSettingsFromFile(Files::Config() + "plugins.txt");
}



void Plugins::Save()
{
	if(plugins.empty())
		return;
	DataWriter out(Files::Config() + "plugins.txt");

	out.Write("state");
	out.BeginChild();
	{
		for(const auto &it : plugins)
			out.Write(it.first, it.second.currentState);
	}
	out.EndChild();
}



// Whether the path points to a valid plugin.
bool Plugins::IsPlugin(const string &path)
{
	// A folder is a valid plugin if it contains one (or more) of the assets folders.
	// (They can be empty too).
	return Files::Exists(path + "data") || Files::Exists(path + "images") || Files::Exists(path + "sounds");
}



// Returns true if any plugin enabled or disabled setting has changed since
// launched via user preferences.
bool Plugins::HasChanged()
{
	for(const auto &it : plugins)
		if(it.second.enabled != it.second.currentState)
			return true;
	return oldNetworkActivity;
}



bool Plugins::IsInBackground()
{
	return currentBackgroundActivity;
}



// Returns the list of plugins that have been identified by the game.
const Set<Plugin> &Plugins::Get()
{
	return plugins;
}



// Toggles enabling or disabling a plugin for the next game restart.
void Plugins::TogglePlugin(const string &name)
{
	auto *plugin = plugins.Get(name);
	plugin->currentState = !plugin->currentState;
}



future<void> Plugins::Install(string url, string name, std::string version)
{
	oldNetworkActivity = true;
	return async(launch::async, [url, name, version]() noexcept -> void
		{
			++currentBackgroundActivity;

			bool success = PluginHelper::Download(url.c_str(),
				(Files::Plugins() + name + ".zip").c_str());
			if(success)
			{
				success = PluginHelper::ExtractZIP(
					(Files::Plugins() + name + ".zip").c_str(),
					Files::Plugins().c_str(), name + "/");
			}
			Files::Write(Files::Plugins() + name + "/version.txt", version);
			Files::Delete(Files::Plugins() + name + ".zip");
			--currentBackgroundActivity;
		});
}



future<void> Plugins::Update(string url, string name, std::string version)
{
	plugins.Get(name)->version = version;

	Files::DeleteDir((Files::Plugins() + name).c_str());
	return Install(url, name, version);
}
