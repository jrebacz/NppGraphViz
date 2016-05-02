# NppGraphViz
A Notepad++ plugin that sends the current tab's document to a GraphViz preview window.  

# Installation
1. Install Notepad++

    Notepad++ is a powerful text editor that supports plugins such as NppGraphViz.  Go to the Notepad++ homepage at [https://notepad-plus-plus.org](https://notepad-plus-plus.org/).  Download and install the latest version.

2. Install Graphviz

    Graphviz is a collection of progams that make diagrams from a simple text language.  Go to Graphviz's homepage at [www.graphviz.org](http://www.graphviz.org/).  Download and install the latest version.

3. Install NppGraphViz

    Download the NppGraphViz.dll from github at [v1.0.1 NppGraphViz.dll](https://github.com/jrebacz/NppGraphViz/releases/download/v1.0.1/NppGraphViz.dll).  Move NppGraphViz.dll into the plugins\ directory of Notepad++.  On a typical install, the full path would be "C:\Program Files (x86)\Notepad++\plugins\".

4. Start Notepad++

    Start Notepad++.  Click "Plugins" from the file menu, you should see "Graphviz" if the plugin successfully installed.

# Features
* Hit the F9 key to preview or refresh your current tab's document with a selected GraphViz program (e.g. dot.exe).
* Save the image with any of GraphViz's supported file types.

# Example
Suppose you have this text in Notepad++:
```
Digraph g
{
    Stop->Collaborate
    Collaborate->Listen
}
```
The plugin sends the text to GraphViz, and displays the following preview:

![graphvizstopcollaboratelisten](https://cloud.githubusercontent.com/assets/12651687/7903042/9c015e94-0795-11e5-9ac9-975c70113a4e.png)
