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
#include "GraphVizPreview.h"
#include <shlobj.h>
#include <windowsx.h>   // for mouse wheel
#include "resource.h" // To get dialog enumerations.

#include <algorithm>

#include "globals.h"

#include "GetGraphvizPath.h"

void Init(HWND)
{
}

/****************************************************************************
* WndProc()
*
*
****************************************************************************/

static UINT lastLayoutEngineCode = ID_LAYOUTENGINE_DOT;
static std::wstring lastLayoutEngine = TEXT("dot.exe");
static void SelectLayoutEngine(UINT check_item)
{
	HMENU mainMenu = ::GetMenu(getGraphVizPreview()->m_hDlg);
	// Deselect everything.
	for (UINT i = ID_LAYOUTENGINE_DOT; i <= ID_LAYOUTENGINE_CIRCO; ++i)
	{ 
		CheckMenuItem(mainMenu, i, MF_BYCOMMAND | MF_UNCHECKED);
	}
	// Select chosen.
	CheckMenuItem(mainMenu, check_item, MF_BYCOMMAND | MF_CHECKED);
    lastLayoutEngineCode = check_item;
}

static void SelectLayoutEngine(UINT check_item, HWND hDlg)
{
    HMENU mainMenu = ::GetMenu(hDlg);
    // Deselect everything.
    for (UINT i = ID_LAYOUTENGINE_DOT; i <= ID_LAYOUTENGINE_CIRCO; ++i)
    {
        CheckMenuItem(mainMenu, i, MF_BYCOMMAND | MF_UNCHECKED);
    }
    // Select chosen.
    CheckMenuItem(mainMenu, check_item, MF_BYCOMMAND | MF_CHECKED);
    lastLayoutEngineCode = check_item;
}

#define CUSTOM_DISABLE_SAVE     50000
#define CUSTOM_ENABLE_SAVE      50001

INT_PTR CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    typedef std::pair<std::wstring, UINT> layout;
    std::vector< layout > layouts;
    layouts.push_back(layout(TEXT("dot.exe"), ID_LAYOUTENGINE_DOT));
    layouts.push_back(layout(TEXT("neato.exe"), ID_LAYOUTENGINE_NEATO));
    layouts.push_back(layout(TEXT("fdp.exe"), ID_LAYOUTENGINE_FDP));
    layouts.push_back(layout(TEXT("sfdp.exe"), ID_LAYOUTENGINE_SFDP));
    layouts.push_back(layout(TEXT("twopi.exe"), ID_LAYOUTENGINE_TWOPI));
    layouts.push_back(layout(TEXT("circo.exe"), ID_LAYOUTENGINE_CIRCO));

    static bool is_dragging = false;

    switch (uMsg)
	{

    case WM_INITDIALOG:
    {
        Init(hwnd);

        ConfigSettings settings = cfg.load();
        if (!settings.graphviz_layout.empty())
        {
            for (auto it = layouts.begin(); it != layouts.end(); ++it)
            {
                if (it->first.find(settings.graphviz_layout) == 0
                    &&
                    settings.graphviz_layout.length() == it->first.length())
                {
                    lastLayoutEngineCode = it->second;
                    lastLayoutEngine = it->first;
                }
            }
        }

        SelectLayoutEngine(lastLayoutEngineCode, hwnd);
		break;
    }
    case WM_PAINT:
    {
		getGraphVizPreview()->draw();
		break;
    }
    case WM_SIZE:
        RedrawWindow(getGraphVizPreview()->m_hDlg, NULL, NULL, RDW_INVALIDATE);
        break;
    case WM_EXITSIZEMOVE:
        RedrawWindow(getGraphVizPreview()->m_hDlg, NULL, NULL, RDW_INVALIDATE);
        break;
    case WM_MOUSEWHEEL:
    {   
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);

        GraphVizPreview * pPreviewWin = getGraphVizPreview();
        
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);

        if (delta > 0)
            pPreviewWin->zoom(x, y, 1.5);
        else
            pPreviewWin->zoom(x, y, 0.5);

        RedrawWindow(pPreviewWin->m_hDlg, NULL, NULL, RDW_INVALIDATE);
        break;
    }
    case WM_MBUTTONDOWN:
        getGraphVizPreview()->reset_zoom();
        RedrawWindow(getGraphVizPreview()->m_hDlg, NULL, NULL, RDW_INVALIDATE);
        break;
    case WM_MOUSEMOVE:
        
        if (wParam & MK_LBUTTON || wParam & MK_RBUTTON) // is left/right mouse button down?
        {
            static int x0, y0;
            
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            if (!is_dragging)
            {
                OutputDebugString(L"start drag");
                is_dragging = true;
                x0 = x;
                y0 = y;
            }

            wchar_t sz[123];
            swprintf(sz, L"%d %d\n", x - x0, y - y0);
            OutputDebugString(sz);

            getGraphVizPreview()->drag(x0-x, y0-y);
            RedrawWindow(getGraphVizPreview()->m_hDlg, NULL, NULL, RDW_INVALIDATE);

            x0 = x;
            y0 = y;
        }
        else
        {
            if (is_dragging)
                OutputDebugString(L"end drag");
            is_dragging = false;
        }
        break;
    case WM_ERASEBKGND:
        return TRUE;    // to reduce flickering when dragging the preview
	case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_SAVE:
            if (!getGraphVizPreview()->m_save_as_path.empty() && !getGraphVizPreview()->m_save_as_ext.empty())
            {
                getGraphVizPreview()->graph(true);
                break;
            }
            //else, fall through to saveas
        case ID_FILE_SAVEAS:
            if(getGraphVizPreview()->saveAs())
            {
                getGraphVizPreview()->graph(true);
            }
            break;
        case ID_FILE_EXIT:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;

            // layout engine switch.
        case ID_LAYOUTENGINE_DOT:
            SelectLayoutEngine(LOWORD(wParam));
            lastLayoutEngine = getGraphVizPreview()->m_layout_engine = TEXT("dot.exe");
            getGraphVizPreview()->refresh();
            break;
        case ID_LAYOUTENGINE_NEATO:
            SelectLayoutEngine(LOWORD(wParam));
            lastLayoutEngine = getGraphVizPreview()->m_layout_engine = TEXT("neato.exe");
            getGraphVizPreview()->refresh();
            break;
        case ID_LAYOUTENGINE_FDP:
            SelectLayoutEngine(LOWORD(wParam));
            lastLayoutEngine = getGraphVizPreview()->m_layout_engine = TEXT("fdp.exe");
            getGraphVizPreview()->refresh();
            break;
        case ID_LAYOUTENGINE_SFDP:
            SelectLayoutEngine(LOWORD(wParam));
            lastLayoutEngine = getGraphVizPreview()->m_layout_engine = TEXT("sfdp.exe");
            getGraphVizPreview()->refresh();
            break;
        case ID_LAYOUTENGINE_TWOPI:
            SelectLayoutEngine(LOWORD(wParam));
            lastLayoutEngine = getGraphVizPreview()->m_layout_engine = TEXT("twopi.exe");
            getGraphVizPreview()->refresh();
            break;
        case ID_LAYOUTENGINE_CIRCO:
            SelectLayoutEngine(LOWORD(wParam));
            lastLayoutEngine = getGraphVizPreview()->m_layout_engine = TEXT("circo.exe");
            getGraphVizPreview()->refresh();
            break;
        case IDOK:
        case IDCANCEL:
            SendMessage(hwnd, WM_CLOSE, 0, 0);
            return TRUE;

            // custom signals not generated from UI
        case CUSTOM_DISABLE_SAVE:
        {
            HMENU mainMenu = ::GetMenu(hwnd);
            EnableMenuItem(mainMenu, ID_FILE_SAVE, 1);
            EnableMenuItem(mainMenu, ID_FILE_SAVEAS, 1);
            break;
        }
        case CUSTOM_ENABLE_SAVE:
        {
            HMENU mainMenu = ::GetMenu(hwnd);
            EnableMenuItem(mainMenu, ID_FILE_SAVE, 0);
            EnableMenuItem(mainMenu, ID_FILE_SAVEAS, 0);
            break;
        }
    }
    break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return TRUE;

    case WM_DESTROY:
        {
            ConfigSettings settings = cfg.load();
            if (!lastLayoutEngine.empty())
                settings.graphviz_layout = lastLayoutEngine;
            cfg.save(settings);
		    //PostQuitMessage(0);
		    killGraphVizPreview();
		    return TRUE;
        }

	}

	return FALSE;
}

GraphVizPreview::GraphVizPreview(HINSTANCE hInst, HWND hWnd)
    : m_b_err(false), m_layout_engine(lastLayoutEngine), m_graphviz_path(TEXT("")), m_zoom(-1.0)
{
	m_hInst = hInst;

	m_hDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, WndProc);
	if (m_hDlg == NULL) 
	{
		MessageBox(hWnd, L"Could not create dialog", 0, 0);
		return;
	}

	ShowWindow(m_hDlg, SW_SHOW);
}


GraphVizPreview::~GraphVizPreview()
{
    lastLayoutEngine = this->m_layout_engine;
}

void GraphVizPreview::zoom(int x, int y, double zoom_amount)
{
    m_zoom *= zoom_amount;

    if (m_output_dimensions.top != 0 || m_output_dimensions.left != 0)
    {
        RECT Rect;
        HWND pImage = GetDlgItem(m_hDlg, ID_PICTURE);
        GetWindowRect(pImage, &Rect);
        // Make x,y relative to pImage top-left
        x -= Rect.left;
        y -= Rect.top;

        double width = (m_output_dimensions.right - m_output_dimensions.left);
        double height = (m_output_dimensions.bottom - m_output_dimensions.top);

        double left_weight = double(x - m_output_dimensions.left) / width;
        double top_weight = double(y - m_output_dimensions.top) / height;

        left_weight = min(left_weight, 1.0);
        left_weight = max(left_weight, 0.0);
        top_weight = min(top_weight, 1.0);
        top_weight = max(top_weight, 0.0);

        double right_weight = 1.0 - left_weight;
        double bottom_weight = 1.0 - top_weight;

        double width0 = (m_original_output_dimensions.right - m_original_output_dimensions.left);
        double height0 = (m_original_output_dimensions.bottom - m_original_output_dimensions.top);

        m_output_dimensions.left = static_cast<LONG>(x - m_zoom * left_weight * width0);
        m_output_dimensions.right = static_cast<LONG>(x + m_zoom * right_weight * width0);
        m_output_dimensions.top = static_cast<LONG>(y - m_zoom * top_weight * height0);
        m_output_dimensions.bottom = static_cast<LONG>(y + m_zoom * bottom_weight * height0);
    }
}

void GraphVizPreview::reset_zoom()
{
    m_zoom = -1.0;
}

void GraphVizPreview::drag(int x, int y)
{
    m_output_dimensions.left -= x;
    m_output_dimensions.top -= y;
    m_output_dimensions.right -= x;
    m_output_dimensions.bottom -= y;
}

void GraphVizPreview::graph(bool saveAs)
{
    wchar_t szCommandLine[300];

    GetGraphvizPath ggp(m_hDlg);
    if (!ggp.setPath(m_graphviz_path))
    {
        m_bmp_data.clear();
        m_b_err = true;
        return;
    }

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;	// So pipe handles are inherited. 
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE g_hChildStd_OUT_Rd, g_hChildStd_OUT_Wr;
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
        throw std::exception("Pipe creation failed.");
    if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
        throw std::exception("Stdout SetHandleInformation");

    HANDLE g_hChildStd_IN_Rd, g_hChildStd_IN_Wr;
    if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
        throw std::exception("Pipe creation failed.");
    if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
        throw std::exception("Stdin SetHandleInformation");

    // Run dot
    STARTUPINFO info;
    PROCESS_INFORMATION processInfo;

    ZeroMemory(&info, sizeof(info));
    ZeroMemory(&processInfo, sizeof(processInfo));;
    info.cb = sizeof(STARTUPINFO);
    info.hStdError = g_hChildStd_OUT_Wr;
    info.hStdOutput = g_hChildStd_OUT_Wr;
    info.hStdInput = g_hChildStd_IN_Rd;
    info.wShowWindow = SW_HIDE;
    info.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    

    m_layout_engine = lastLayoutEngine;
    if (ggp.hasPathEnv())
    {
        if (saveAs)
            wsprintf(szCommandLine, TEXT("%s -T%s -o\"%s\""),
            m_layout_engine.c_str(), m_save_as_ext.c_str(), m_save_as_path.c_str());
        else
            wsprintf(szCommandLine, TEXT("%s -Tbmp"),
            m_layout_engine.c_str());

        if (!CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE,
            CREATE_UNICODE_ENVIRONMENT, (void*)ggp.loadPathVar().c_str(),
            NULL, &info, &processInfo))
        {
            throw std::exception("Error starting GraphViz using PATH.");
        }
    }
    else
    { 
        if (saveAs)
            wsprintf(szCommandLine, TEXT("\"%s%s\" -T%s -o\"%s\""),
            m_graphviz_path.c_str(), m_layout_engine.c_str(), m_save_as_ext.c_str(), m_save_as_path.c_str());
        else
            wsprintf(szCommandLine, TEXT("\"%s%s\" -Tbmp"),
            m_graphviz_path.c_str(), m_layout_engine.c_str());

        if (!CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE,
            0, NULL,
            m_graphviz_path.c_str(), &info, &processInfo))
        {
            if (GetLastError() == ERROR_FILE_NOT_FOUND)
            {
                m_graphviz_path.clear();
                cfg.load().graphviz_path.clear();
            }

            throw std::exception("Error starting GraphViz.");
        }
    }


    DWORD bytes;
    if (!WriteFile(g_hChildStd_IN_Wr, &*m_npp_text.begin(), static_cast<DWORD>(m_npp_text.size()), &bytes, NULL))
    {
        throw std::exception("Error writing to child process.");
    }
    // write EOF (this may not be necessary)
    const char byte_eof = EOF;
    if (!WriteFile(g_hChildStd_IN_Wr, &byte_eof, 1, &bytes, NULL))
    {
        throw std::exception("Error writing to child process.");
    }
    CloseHandle(g_hChildStd_IN_Wr);

    std::vector<char> outBmp(0);

    DWORD total_available, bytes_left_this_message;
    for (bool last_read = false;;)
    {
        // Check for program output.
        if (PeekNamedPipe(g_hChildStd_OUT_Rd, NULL, 0, NULL, &total_available, &bytes_left_this_message) == 0)
        {
            break;
        }

        if (total_available == 0)
        {
            // Is the process done?
            if (::WaitForSingleObject(processInfo.hProcess, 0 /*milliseconds*/) != WAIT_TIMEOUT)
            {
                if (last_read)
                    break;
                last_read = true;
                continue;
            }
            // Wait more.
            Sleep(5);
            continue;
        }

        // Read BMP data.
        const size_t prev_size = outBmp.size();
        outBmp.resize(prev_size + total_available);

        if (ReadFile(g_hChildStd_OUT_Rd, &outBmp[prev_size], total_available, &bytes, NULL) == 0)
        {
            break;
        }
    }

    DWORD exit_code;
    GetExitCodeProcess(processInfo.hProcess, &exit_code);

    ::WaitForSingleObject(processInfo.hProcess, INFINITE);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    CloseHandle(g_hChildStd_OUT_Rd);

    if (exit_code)
    {
        SendMessage(m_hDlg, WM_COMMAND, CUSTOM_DISABLE_SAVE, 0);
    }
    else
    {
        SendMessage(m_hDlg, WM_COMMAND, CUSTOM_ENABLE_SAVE, 0);
        lastLayoutEngine = m_layout_engine;
    }

    if (!saveAs)
    {
        m_bmp_data = outBmp;
        m_b_err = exit_code != 0;
    }
    else
    {
        if (exit_code)
        {
            ::MessageBoxA(NULL, "Failed to save graph!", "", MB_OK);
        }
    }
}

void GraphVizPreview::draw()
{
    if (m_bmp_data.empty())
        return;

    HWND pImage = GetDlgItem(m_hDlg, ID_PICTURE);

    {
        HWND pEdit = GetDlgItem(m_hDlg, IDC_GVP_EDIT);
        if (m_b_err)
        {
            const int offset = 40;
            SetWindowPos(m_hDlg, NULL, 0, 0, 600, 400, SWP_NOMOVE);
            SetWindowPos(pEdit, NULL, 0, 0, 600 - offset, 400 - offset, SWP_NOMOVE);
            m_bmp_data.push_back('\0');
            SetWindowTextA(pEdit, &m_bmp_data[0]);
            ShowWindow(pEdit, SW_SHOWNORMAL);
            EnableWindow(pEdit, TRUE);

            ShowWindow(pImage, SW_HIDE);
            EnableWindow(pImage, FALSE);

            InvalidateRect(m_hDlg, NULL, TRUE);
            return;
        }
        else
        {
            SetWindowTextA(pEdit, "");
            ShowWindow(pEdit, SW_HIDE);
            EnableWindow(pEdit, FALSE);

            ShowWindow(pImage, SW_SHOWNORMAL);
            EnableWindow(pImage, TRUE);
        }
    }

    HBITMAP hBmp;
    {
        char *pBuffer = &*m_bmp_data.begin();
        tagBITMAPFILEHEADER bfh = *(tagBITMAPFILEHEADER*)pBuffer;
        tagBITMAPINFOHEADER bih = *(tagBITMAPINFOHEADER*)(pBuffer + sizeof(tagBITMAPFILEHEADER));
        RGBQUAD             rgb = *(RGBQUAD*)(pBuffer + sizeof(tagBITMAPFILEHEADER) + sizeof(tagBITMAPINFOHEADER));

        BITMAPINFO bi;
        bi.bmiColors[0] = rgb;
        bi.bmiHeader = bih;

        char* pPixels = (pBuffer + bfh.bfOffBits);
        char* ppvBits = nullptr;

        hBmp = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (void**)&ppvBits, NULL, 0);
        SetDIBits(NULL, hBmp, 0, bih.biHeight, pPixels, &bi, DIB_RGB_COLORS);
    }

    BITMAP 			bitmap;
    GetObject(hBmp, sizeof(bitmap), &bitmap);
    // Resize
    RECT dlg_dimensions, output_dimensions;
    GetClientRect(m_hDlg, &dlg_dimensions);

    // Maximize the draw area to the available area.
    SetWindowPos(pImage, NULL, 0, 0, dlg_dimensions.right, dlg_dimensions.bottom, SWP_NOMOVE | SWP_NOACTIVATE);

    // We will have to scale the bitmap, this section applies the scaling factor
    output_dimensions = dlg_dimensions;
    output_dimensions.right -= 1;
    output_dimensions.bottom -= 1;

    m_original_output_dimensions.top = 0;
    m_original_output_dimensions.left = 0;
    m_original_output_dimensions.right = bitmap.bmWidth;
    m_original_output_dimensions.bottom = bitmap.bmHeight;
    if (m_zoom <= 0.0)
    {
        double width_ratio = double(output_dimensions.right) / bitmap.bmWidth;
        double height_ratio = double(output_dimensions.bottom) / bitmap.bmHeight;

        if (width_ratio < 1.0 || height_ratio < 1.0)
        {
            if (width_ratio < height_ratio)
            {
                output_dimensions.right = static_cast<LONG>(bitmap.bmWidth * width_ratio);
                output_dimensions.bottom = static_cast<LONG>(bitmap.bmHeight * width_ratio);
                m_zoom = width_ratio;
            }
            else
            {
                output_dimensions.right = static_cast<LONG>(bitmap.bmWidth * height_ratio);
                output_dimensions.bottom = static_cast<LONG>(bitmap.bmHeight * height_ratio);
                m_zoom = height_ratio;
            }
        }
        else
        {
            output_dimensions.right = bitmap.bmWidth;
            output_dimensions.bottom = bitmap.bmHeight;
            m_zoom = 1.0;
        }
    }
    else
    {
        m_output_dimensions.right = static_cast<LONG>(m_output_dimensions.left + m_zoom * bitmap.bmWidth);
        m_output_dimensions.bottom = static_cast<LONG>(m_output_dimensions.top + m_zoom * bitmap.bmHeight);
        output_dimensions = m_output_dimensions;
    }

    m_output_dimensions = output_dimensions;

    // Begin painting    
	PAINTSTRUCT 	ps;
	HDC 			hdc = BeginPaint(pImage, &ps);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HGDIOBJ oldBitmap = SelectObject(hdcMem, hBmp);

    // Draw dark grey everywhere
    HBRUSH bg_color = CreateSolidBrush(RGB(200,200,200));
    FillRect(hdc, &dlg_dimensions, bg_color);
    DeleteObject(bg_color);

    // Copy from bitmap to dialog rectangle, with scaling.
    StretchBlt(hdc, 
        output_dimensions.left, output_dimensions.top,
        static_cast<LONG>(bitmap.bmWidth * m_zoom),
        static_cast<LONG>(bitmap.bmHeight * m_zoom),
        hdcMem,
        0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

	SelectObject(hdcMem, oldBitmap);

	DeleteDC(hdcMem);

	EndPaint(pImage, &ps);

	DeleteObject(hBmp);
	
}

void GraphVizPreview::refresh()
{
	launchGraphVizPreview();
}

bool GraphVizPreview::saveAs()
{
    OPENFILENAME ofn = { 0 };

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = m_hDlg;
    ofn.lpstrFilter =
        TEXT("BMP (Windows Bitmap Format) (*.bmp)\0*.bmp\0")
        TEXT("DOT (*.canon)\0*.canon\0")
        TEXT("DOT (*.dot)\0*.dot\0")
        TEXT("DOT (*.gv)\0*.gv\0")
        TEXT("DOT (*.xdot)\0*.xdot\0")
        TEXT("DOT (*.xdot1.2)\0*.xdot1.2\0")
        TEXT("DOT (*.xdot1.4)\0*.xdot1.4\0")
        TEXT("CGImage bitmap format (*.cgimage)\0*.cgimage\0")
        TEXT("CMAP (Client-side imagemap) (deprecated) (*.cmap)\0*.cmap\0")
        TEXT("EPS (Encapsulated PostScript) (*.eps)\0*.eps\0")
        TEXT("EXR (OpenEXR) (*.exr)\0*.exr\0")
        TEXT("FIG (*.fig)\0*.fig\0")
        TEXT("GD (*.gd)\0*.gd\0")
        TEXT("GD2 (*.gd2)\0*.gd2\0")
        TEXT("GIF (*.gif)\0*.gif\0")
        TEXT("GTK canvas (*.gtk)\0*.gtk\0")
        TEXT("ICO (Icon Image File Format) (*.ico)\0*.ico\0")
        TEXT("IMAP (Server-side imagemap) (*.imap)\0*.imap\0")
        TEXT("CMAPX (Client-side imagemap) (*.cmapx)\0*.cmapx\0")
        TEXT("IMAP_NP (Server-side imagemap) (*.imap_np)\0*.imap_np\0")
        TEXT("CMAPX_NP (Client-side imagemap) (*.cmapx_np)\0*.cmapx_np\0")
        TEXT("ISMAP (Server-side imagemap (deprecated)) (*.ismap)\0*.ismap\0")
        TEXT("JPEG 2000 (*.jp2)\0*.jp2\0")
        TEXT("JPEG (*.jpg)\0*.jpg\0")
        TEXT("JPEG (*.jpeg)\0*.jpeg\0")
        TEXT("JPEG (*.jpe)\0*.jpe\0")
        TEXT("PICT (*.pict)\0*.pict\0")
        TEXT("PDF (Portable Document Format) (PDF) (*.pdf)\0*.pdf\0")
        TEXT("PIC (Kernighan's PIC graphics language) (*.pic)\0*.pic\0")
        TEXT("PLAIN (Simple text format) (*.plain)\0*.plain\0")
        TEXT("PLAIN-EXT (Simple text format) (*.plain-ext)\0*.plain-ext\0")
        TEXT("PNG (Portable Network Graphics format) (*.png)\0*.png\0")
        TEXT("POV-Ray markup language (prototype) (*.pov)\0*.pov\0")
        TEXT("PS (PostScript) (*.ps)\0*.ps\0")
        TEXT("PS2 (PostScript) for PDF (*.ps2)\0*.ps2\0")
        TEXT("PSD (*.psd)\0*.psd\0")
        TEXT("SGI (*.sgi)\0*.sgi\0")
        TEXT("SVG (Scalable Vector Graphics) (*.svg)\0*.svg\0")
        TEXT("SVGZ (Scalable Vector Graphics) (*.svgz)\0*.svgz\0")
        TEXT("TGA (Truevision TGA) (*.tga)\0*.tga\0")
        TEXT("TIFF (Tag Image File Format) (*.tif)\0*.tif\0")
        TEXT("TIFF (Tag Image File Format) (*.tiff)\0*.tiff\0")
        TEXT("TK graphics (*.tk)\0*.tk\0")
        TEXT("VML (Vector Markup Language) (*.vml)\0*.vml\0")
        TEXT("VML (Vector Markup Language) (*.vmlz)\0*.vmlz\0")
        TEXT("VRML (*.vrml)\0*.vrml\0")
        TEXT("WBMP (Wireless BitMap format) (*.wbmp)\0*.wbmp\0")
        TEXT("WEBP (Image format for the Web) (*.webp)\0*.webp\0")
        TEXT("Xlib canvas (*.xlib)\0*.xlib\0")
        TEXT("Xlib canvas (*.x11)\0*.x11\0")
        TEXT("\0\0");// ends the pairs
        
    ConfigSettings &settings = cfg.load();

    static TCHAR initial_directory[MAX_PATH];
    lstrcpynW(initial_directory, settings.save_as_path.c_str(), MAX_PATH);

    TCHAR szFile[MAX_PATH] = { 0 };
    TCHAR szFileTitle[MAX_PATH] = { 0 };

    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(*szFile);
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = sizeof(szFileTitle);
    ofn.lpstrInitialDir = initial_directory;
    ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
    ofn.lpstrTitle = TEXT("Save As");
    ofn.nFilterIndex = settings.save_as_filter_index;
    
    ofn.Flags |= OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_ENABLESIZING;

    //ofn.Flags |= OFN_ENABLEHOOK;
    //ofn.lpfnHook = OFNHookProc;

    try {
        if (::GetSaveFileName((OPENFILENAME*)&ofn) == false)
            return false;

        // Main things to do next:
        // 1. Save filter index and directory for next Save As.
        // 2. Adjust the file name:
        //      * if no extension, use the filter extension.
        //      * if the file_name contains an extension that differs
        //          from the filter extension, change the filter extension.
        // 3. Store the save as path and extension for Graphviz.
        
        // 1.  Save filter index and directory for next Save As.
        settings.save_as_filter_index = ofn.nFilterIndex; // to save for next Save As.

        // Use the filter index to find the filter string.
        int file_type_inx = 0;
        for (DWORD i = 1; i < ofn.nFilterIndex * 2; ++i)
        {
            if (ofn.lpstrFilter[file_type_inx] == '\0')
                return false;
            while (ofn.lpstrFilter[file_type_inx++] != '\0'){}
        }

        std::wstring file_directory(ofn.lpstrFile);
        std::wstring file_type(&ofn.lpstrFilter[file_type_inx]);
        file_type = file_type.substr(2); // remove the "*.", first two characters.
        
        // Store the initial directory for next Save As.
        std::wstring::size_type dir_slash = file_directory.rfind(TEXT("\\"));
        std::wstring file_name = file_directory.substr(dir_slash);
        file_directory = file_directory.substr(0, dir_slash);
        settings.save_as_path = file_directory;

        // 2. Adjust the file name:
        
        //If the file_name doesn't contain an extension, append the extension we know.
        if (file_name.find(TEXT(".")) == std::wstring::npos)
        {
            file_name.append(TEXT("."));
            file_name.append(file_type);
        }
        else
        {
            // Is the extension different from our filter?
            if (file_name.rfind(file_type) == std::wstring::npos || !(file_name.rfind(file_type) == file_name.size() - file_type.size()))
            {
                std::wstring::size_type pos = file_name.rfind(TEXT("."));
                if (pos != std::wstring::npos)
                {
                    file_type = file_name.substr(pos + 1);
                }
            }
        }

        // 3. Store the "save as" path and extension for Graphviz.
        m_save_as_path = file_directory + file_name;
        m_save_as_ext = file_type;

        return true;
    }
    catch (std::exception e) {
        ::MessageBoxA(NULL, e.what(), "Exception", MB_OK);
    }
    catch (...) {
        ::MessageBox(NULL, TEXT("GetSaveFileName crashes!!!"), TEXT(""), MB_OK);
    }

    return false;
}

static GraphVizPreview *graphVizDlg;

GraphVizPreview * getGraphVizPreview()
{
	if (graphVizDlg == nullptr)
	{
		int handleForGraphViz = 0;
		::SendMessage(nppData._nppHandle, NPPM_MODELESSDIALOG, MODELESSDIALOGADD, (LPARAM)&handleForGraphViz);

		graphVizDlg = new GraphVizPreview(hInst, nppData._nppHandle);

	}	
	return graphVizDlg;
}

void killGraphVizPreview()
{
	delete graphVizDlg;
	graphVizDlg = nullptr;
}