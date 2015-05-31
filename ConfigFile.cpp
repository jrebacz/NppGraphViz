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
#include "ConfigFile.h"

#include "PluginInterface.h"
#include "globals.h"
#include <Shlwapi.h>

ConfigFile::ConfigFile()
{
}


ConfigFile::~ConfigFile()
{
}

std::wstring ConfigFile::getConfPath()
{
    TCHAR confDir[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)confDir);
    std::wstring confPath;
    confPath = confDir;
    confPath += TEXT("\\");
    confPath += TEXT("Graphviz");
    confPath += TEXT(".ini");
    return confPath;
}

void ConfigFile::save(const ConfigSettings &settings)
{
    m_settings = settings;
    WritePrivateProfileString(TEXT("Graphviz"), TEXT("Path"), m_settings.graphviz_path.c_str(), getConfPath().c_str());
    WritePrivateProfileString(TEXT("Graphviz"), TEXT("Layout"), m_settings.graphviz_layout.c_str(), getConfPath().c_str());
    WritePrivateProfileString(TEXT("Graphviz"), TEXT("SaveAsPath"), m_settings.save_as_path.c_str(), getConfPath().c_str());

    TCHAR sz[MAX_PATH];
    swprintf(sz, MAX_PATH, TEXT("%d"), m_settings.save_as_filter_index);
    WritePrivateProfileString(TEXT("Graphviz"), TEXT("SaveAsFilterIndex"), sz, getConfPath().c_str());
}

ConfigSettings& ConfigFile::load()
{
    if (m_is_loaded)
        return m_settings;

    std::wstring confPath = getConfPath();
    const char confContent[] =
"\
; This file stores parameters for the NppGraphViz plugin.  Be careful when\n\
; modifying.  This file is auto-created by the plugin, so to revert back to\n\
; the default settings, just delete this file and restart Notepad++. \n\
;\n\
; *** PREREQUISITE ***\n\
; A graphviz installation is required for this plugin to work.  Graphviz may\n\
; be downloaded at http://graphviz.org/\n\
;\n\
; * Path: leave blank if GraphViz's bin folder (where dot.exe lives) is in\n\
;       your environment's PATH variable.  Otherwise, this parameter should\n\
;       be set to the full path of GraphViz's bin folder.\n\
; * Layout: the layout program (such as dot.exe) that generates images of\n\
;       graphs from code.\n\
; * SaveAsPath: the last directory an image was saved to.\n\
; * SaveAsFilterIndex: an integer that marks the image's last saved type.\n\
[Graphviz]\n\
Path=\n\
Layout=dot.exe\n\
SaveAsPath=\n\
SaveAsFilterIndex=0\n\
\n";

    if (!::PathFileExists(confPath.c_str()))
    {
        // Create the file since it doesn't exist.
        FILE *f = _wfopen(confPath.c_str(), TEXT("w"));
        if (f)
        {
            fwrite(confContent, sizeof(confContent[0]), strlen(confContent), f);
            fclose(f);
        }
    }

    TCHAR cmdNames[MAX_PATH];
    ::GetPrivateProfileSectionNames(cmdNames, MAX_PATH, confPath.c_str());

    if (wcscmp(cmdNames, TEXT("Graphviz")) == 0)
    {
        TCHAR str[MAX_PATH];
        int val = GetPrivateProfileString(cmdNames, TEXT("Path"), 0, str, MAX_PATH, confPath.c_str());
        if (val)
        {
            m_settings.graphviz_path = str;
        }

        val = GetPrivateProfileString(cmdNames, TEXT("Layout"), 0, str, MAX_PATH, confPath.c_str());
        if (val)
        {
            m_settings.graphviz_layout = str;
        }

        val = GetPrivateProfileString(cmdNames, TEXT("SaveAsPath"), 0, str, MAX_PATH, confPath.c_str());
        if (val)
        {
            m_settings.save_as_path = str;
        }

        m_settings.save_as_filter_index = GetPrivateProfileInt(cmdNames, TEXT("SaveAsFilterIndex"), 0, confPath.c_str());
    }

    m_is_loaded = true;

    return m_settings;
}

ConfigFile cfg;