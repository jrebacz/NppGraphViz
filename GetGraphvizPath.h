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

#include <shlobj.h>
#include <Shlwapi.h>

#include "ConfigFile.h"

// This class determines if GraphViz's path is set.
class GetGraphvizPath
{
public:

    GetGraphvizPath(HWND parent)
        :m_parent(parent), m_hasPathEnum(HPE_UNKNOWN)
    {
    }

    ~GetGraphvizPath()
    {
    }

    std::wstring loadPathVar()
    {
        TCHAR* path_var;
        size_t requiredSize;

        _wgetenv_s(&requiredSize, NULL, 0, TEXT("PATH"));
        if (requiredSize == 0)
        {
            return std::wstring();
        }

        path_var = (TCHAR*)malloc(requiredSize * sizeof(TCHAR));
        if (!path_var)
        {
            return std::wstring();
        }

        // Get the value of the LIB environment variable.
        _wgetenv_s(&requiredSize, path_var, requiredSize, TEXT("PATH"));

        std::wstring str;
        str.append(TEXT("PATH="));
        str.append(path_var);
        str.push_back(TEXT('\0')); // I think lpEnvironment needs 2 nul chars to terminate.


        free(path_var);
        return str;
    }

    bool hasPathEnv()
    {
        if (m_hasPathEnum != HPE_UNKNOWN)
        {
            return m_hasPathEnum == HPE_YES;
        }

        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;	// So pipe handles are inherited. 
        saAttr.lpSecurityDescriptor = NULL;

        // Run dot
        STARTUPINFO info;
        PROCESS_INFORMATION processInfo;

        ZeroMemory(&info, sizeof(info));
        ZeroMemory(&processInfo, sizeof(processInfo));;
        info.cb = sizeof(STARTUPINFO);
        info.hStdError = NULL;
        info.hStdOutput = NULL;
        info.hStdInput = NULL;
        info.wShowWindow = SW_HIDE;
        info.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        
        std::wstring path_env(loadPathVar());

        wchar_t szCommandLine[300];
        wsprintf(szCommandLine, TEXT("dot.exe -V"));
        int ret = CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE,
            CREATE_UNICODE_ENVIRONMENT, (void*)path_env.c_str(),
            NULL, &info, &processInfo);
        if (ret)
        {
            ::WaitForSingleObject(processInfo.hProcess, 0 /*milliseconds*/);
            // CreateProcess succeeded
            m_hasPathEnum = HPE_YES;
            // dot was found
            return true;
        }

        m_hasPathEnum = HPE_NO;

        //dot wasn't found
        ret = GetLastError();
        if (ret == ERROR_FILE_NOT_FOUND)
        {
            return false;
        }

        // wierd case
        _ASSERT(0);
        return false;
    }

    void appendTrailingSlash(std::wstring &path)
    {
        if (*path.rbegin() == TEXT('\\') || *path.rbegin() == TEXT('/'))
            return;
        path.push_back(TEXT('\\'));
    }

    bool setPath(std::wstring &graphviz_path)
    {
        ConfigSettings &settings = cfg.load();
        
        if (!settings.graphviz_path.empty())
        {
            // User is not using the environmental PATH var.
            graphviz_path = settings.graphviz_path;
            appendTrailingSlash(graphviz_path);
            return true;
        }

        if (hasPathEnv())
        {
            graphviz_path.clear();
            return true;
        }

        int ret = 
        ::MessageBox(m_parent, TEXT("Cannot find Graphviz on PATH.  Choose OK to select Graphviz's installation ")
            TEXT("directory.  Choose Cancel if you would like a moment to add Graphviz's installation directory ")
            TEXT("to PATH."),
            TEXT("Graphviz plugin cannot find graphviz"), MB_OKCANCEL);
        if (ret != IDOK)
        {
            return false;
        }
        else
        {
            bool success = false;
            LPMALLOC pShellMalloc = 0;
            if (::SHGetMalloc(&pShellMalloc) == NO_ERROR)
            {
                // If we were able to get the shell malloc object,
                // then proceed by initializing the BROWSEINFO stuct
                BROWSEINFO info;
                memset(&info, 0, sizeof(info));
                info.hwndOwner = m_parent;
                info.pidlRoot = NULL;
                TCHAR szDisplayName[MAX_PATH];
                info.pszDisplayName = szDisplayName;
                info.lpszTitle = TEXT("Select Graphviz's installation directory.");
                info.ulFlags = 0;
                info.lpfn = 0;
                TCHAR directory[MAX_PATH];

                if (!directory[0] && m_defaultDirectory)
                    info.lParam = reinterpret_cast<LPARAM>(m_defaultDirectory);
                else
                    info.lParam = reinterpret_cast<LPARAM>(directory);

                // Execute the browsing dialog.
                LPITEMIDLIST pidl = ::SHBrowseForFolder(&info);

                // pidl will be null if they cancel the browse dialog.
                // pidl will be not null when they select a folder.
                if (pidl)
                {
                    // Try to convert the pidl to a display generic_string.
                    // Return is true if success.
                    TCHAR szDir[MAX_PATH];
                    if (::SHGetPathFromIDList(pidl, szDir))
                    {
                        graphviz_path = szDir;
                        appendTrailingSlash(graphviz_path);
                        settings.graphviz_path = graphviz_path;

                        cfg.save(settings);

                        success = true;
                    }
                    pShellMalloc->Free(pidl);
                }
                pShellMalloc->Release();
            }
            return success;
        }

        return true;
    }
private:
    HWND m_parent;
    TCHAR m_defaultDirectory[MAX_PATH];
    enum HasPathEnum {HPE_UNKNOWN, HPE_YES, HPE_NO};
    HasPathEnum m_hasPathEnum;
};

