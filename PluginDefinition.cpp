//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
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

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include "GraphVizPreview.h"
#include "globals.h"
#include <vector>
#include <fstream>

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;
HINSTANCE hInst;
//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
	hInst = (HINSTANCE)hModule;
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    static ShortcutKey gviz_short_cut_refresh = { false, false, false, VK_F9 };
	
    setCommand(0, TEXT("Preview/Refresh"), launchGraphVizPreview, &gviz_short_cut_refresh, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
	if (index >= nbFunc)
		return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//


// Reads the text of the currently opened tab into a char vector.
 std::vector<char> getDocument()
{
	std::vector<char> doc_text;
	// Get the current scintilla
	int which = -1;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
	if (which == -1)
		return doc_text;
	HWND curScintilla = (which == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;

	int doc_length = static_cast<int>(::SendMessage(curScintilla, SCI_GETLENGTH, 0, 0));
	if (doc_length < 2)
		return doc_text;
	doc_text.resize(doc_length + 1);
	// Scintilla control has no Unicode mode, so we use (char *) here
	::SendMessage(curScintilla, SCI_GETTEXT, doc_length + 1, (LPARAM)&*doc_text.begin());

	return doc_text;
}

void launchGraphVizPreview()
{
    try
    {
        std::vector<char> doc_text = getDocument();
        if (doc_text.empty())
        {
            ::MessageBox(NULL, TEXT("The document in the current tab is empty.  Can't send to Graphviz"), TEXT("Graphviz Plugin Error"), MB_OK | MB_APPLMODAL);
            return;
        }

        GraphVizPreview * gvp = getGraphVizPreview();
        gvp->m_npp_text = doc_text;
        gvp->graph(false);
        RedrawWindow(gvp->m_hDlg, NULL, NULL, RDW_INVALIDATE);
    }
    catch (std::exception &e)
    {
        ::MessageBoxA(NULL, e.what(), "Graphviz Plugin Error", MB_OK | MB_APPLMODAL);
    }
}
