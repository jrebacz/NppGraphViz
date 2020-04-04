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

#include <Windows.h>

#include <vector>
#include <xstring>

class GraphVizPreview
{
public:
    GraphVizPreview(HINSTANCE, HWND);
    ~GraphVizPreview();

    void zoom(int x, int y, double zoom_amount);
    void reset_zoom();
    void drag(int x, int y);

    void graph(bool saveAs);
    std::vector<char> readFromPipe(HANDLE pipe, PROCESS_INFORMATION& process_info, const char* label);
    void draw();
    void refresh();
    bool saveAs();

    std::wstring m_graphviz_path;
    std::wstring m_save_as_path;
    std::wstring m_save_as_ext;

    std::wstring m_layout_engine; // e.g. "dot.exe"
    bool m_b_err;
    std::vector<char> m_npp_text;
    std::vector<char> m_bmp_data;
    
    double m_zoom;
    RECT m_original_output_dimensions;
    RECT m_output_dimensions;

    HWND m_hDlg;
    HINSTANCE m_hInst;
};

GraphVizPreview * getGraphVizPreview();
void launchGraphVizPreview();
void killGraphVizPreview();
