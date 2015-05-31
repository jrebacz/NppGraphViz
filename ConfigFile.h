//This file is part of NppGraphViz.
//Copyright (C)2015 Jeff Rebacz  <jeffrey.rebacz@gmail.com>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
#include <xstring>

// Holds all settings read from and saved to the ConfigFile.
struct ConfigSettings
{
public:
    std::wstring graphviz_path;
    std::wstring graphviz_layout;
    std::wstring save_as_path;
    int save_as_filter_index;
};

// Used to read/write the config file.
class ConfigFile
{
public:
    ConfigFile();
    ~ConfigFile();

    static std::wstring getConfPath();
    void save(const ConfigSettings &settings);
    ConfigSettings& load();

private:
    ConfigSettings m_settings;
    bool m_is_loaded;
};

