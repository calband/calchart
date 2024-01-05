# CalChart

The CalChart program is written in C/C++ using wxWidgets UI Framework to create a cross-platform application. 
CalChart is 3 different things:

 * **Composer**: A tool for allowing members of the Stunt committee to create shows.
 * **Viewer**: A way for Stunt members to view their creation.
 * **Exporter**: A way to export the show in a "learnable" form to members of the band. 

In order to think about the way that CalChart is architected it is helpful to take a moment to consider how a computer would represent a Show for the CalBand.  It would likely have to have an array of Stunt Sheets, each one having an array of positions of each of the Marchers on the field.  Each Sheet would likely have some way to describe the Continuity of a set of Marchers from their current position to their position on the following Stunt Sheet, as well as the number of Beats for that particular sheet.

At the core, CalChart is essentially a "drawing" tool where you draw out the position of Marchers (aka "dots"), and then dictate the way that the Marchers will move between sheets via their continuities.  Internally it maintains data structures that represents those positions through-out a show, and can calculate if there are incoherences such as when Marchers are not able to make it to their positions or would collide with others. 


# Architecture

  *"The type of UI system you have strongly influences the way you program." - Jeff Lee*

While CalChart is written in C++, it's architecture is more due to wxWidgets, which influences the overall structure and composition of the program.  It is advisable to read about wxWidgets at http://wxwidgets.org.

There are different UI windowing systems (like Cocoa for Mac, or GNOME for linux, or QT for cross platform), but wxWidgets was chosen as a good balance of features, cross-platform, and support.  This allows one version to be built on Windows/MacOS/Linux, but also means that proprietary UI Kits like iOS would be difficult to support, requiring a complete rewrite.

## CalChartCore vs wxCalChart

In order to provide a separation between the wxWidgets UI framework part of the project and the generic part of CalChart, we try to maintain a difference between the UI Application specific code (wxCalChart) and the more generic Core functional code (CalChartCore).  Code specific that relies on the wxWidgets framework should not be in Core, and objects that can be pure C++ should be in Core.  This provides a distinction between the "How" and the "What".

For example, "How to draw" a field with marchers on it is very different than the "What and Where to draw".  Drawing a continuity path or a shape can be described as a collection of lines and circles, so CalChartCore there is a `CalChart::Shape` object, which is interpreted by the varous `wxView` draw routines to draw to the screen or to PDF for printing.

We can think of CalChartCore as a portable "library" that could be instantiated in any project to parse show files, to interpret continuities to develop the marcher's paths, and to give data structures that represent where a show layout should be.  A way to think about it is that CalChartCore is basically a `*.shw` to `json` converter -- that you should be able to create a simple commandline tool that wraps the CalChartCore library that reads in a `*.shw` file and then spits out a `json` file that describes where every point should be.

To help with separation of the two parts of the code base, symbols and data structres of CalChartCore should generally be in the `CalChart` namespace.  Symbols, data structures, and general wxWidgets windows and dialogs should be in the `wxCalChart` namespace.  This helps keep the symbols distinct and makes it easier to understand what types of objects the program is using.

## CalChartCore

### `CalChart::Show`

The `CalChart::Show` is a data object that holds the core parts of the Show.  It maintains the list of Marchers labels and instruments for the Show, as well as all the Stunt sheets.  A good model for thinking of the `CalChart::Show` is that it represents all that should be "Saved" and "Loaded" when you want to continue editing a Show.

In principle all details needed to read or display a show can be done through the `CalChart::Show` interface.  For example, to determine what label is assigned to a particular point, you would call the `CalChart::Show::GetPointLabel` function.

Modification of a show (or internal details of a show such as point positions on an individual sheet) should not be done directly by function calls, but instead you should construct an `CalChart::Show_command_pair` object, and then execute the first of the pair.  This feels awkward at first, but this layer greatly enhances the ability to have a Do/Undo system -- by having the modification and "un"-modification occur in pairs, you can always go back to a previous state.

### `CalChart::Animation`

The `CalChart::Animation` is an object that is created from a `CalChart::Show` and represents a fully Animated show.  It is used for the various Animation render views to see a top down or 3D version of the show.  It can also be used to show the paths a Marcher would travel in the Field view.


## wxCalChart

### Document/View

CalChart is a `wxApp` which heavily uses wxWidget's Document/View model described at  https://docs.wxwidgets.org/3.0/overview_docview.html:

> The idea is that you can model your application primarily in terms of documents to store data and provide interface-independent operations upon it, and views to visualise and manipulate the data. Documents know how to do input and output given stream objects, and views are responsible for taking input from physical windows and performing the manipulation on the document data.
> If a document's data changes, all views should be updated to reflect the change. The framework can provide many user-interface elements based on this model.
> Once you have defined your own classes and the relationships between them, the framework takes care of popping up file selectors, opening and closing files, asking the user to save modifications, routing menu commands to appropriate (possibly default) code, even some default print/preview functionality and support for command undo/redo.

The central object that maintains the data model for the CalChart Show is the `CalChartDoc`, which is a subclass of type `wxDocument`.  All modifications to the Show are done via the `CalChartDoc`, and modifications are published out to all the `wxView` objects which read from the `CalChartDoc` to update the various `wxFrame` and `wxWindow` objects that constitute the CalChart Application.  While there are many custom `wxDialog` or custom `wxFrame` objects created to display different data to the user when using the application, they should generally be thought of as *viewing* the `CalChartDoc` for some specific purpose.

### `CalChartDoc`

The `CalChartDoc` is the *document* in the CalChart Document/View model.  It holds the loaded `CalChart::Show`, the corresponding `CalChart::Animation`, as well as any temporary data objects that are needed for maintaining the UI appearance.  All interactions with the `CalChart::Show` object should go through `CalChartDoc`.

Whenever the `CalChart::Show` changes, the `CalChartDoc` will re-create the `CalChart::Animation` so that there is a fresh animation for the various *Views* to use.

### `CalChartView`

The `CalChartView` is the way that the various `wxDialog`, `wxFrame`, or `wxPanel` objects interact with the `CalChartDoc`.  This allows a central place where Drawing and Document manipulation can go through.
A programming paradigm that CalChart frequently uses is to create a wxFrame object and then assign a wxView to that wxFrame.  This allows the frame to be "connected" to the *Document* so that it has a shared "View" into the data model.

### Modification via `wxCommand`

CalChart utilizes the `wxCommand` objects as described in the  [Document/View](https://docs.wxwidgets.org/3.0/overview_docview.html) model:

> When a user interface event occurs, the application submits a command to a wxCommandProcessor object to execute and store.
> The wxWidgets document/view framework handles Undo and Redo by use of wxCommand and wxCommandProcessor objects.

When a *View* wants to modify the CalChart Document, it would do so by creating the appropriate `wxCommand`, and then submits that command to the *Document's* command processor.  This will cause the *Document* to be modified, and all the appropriate *Views* to be updated.  It will also cause the correct "Do" and "Undo" history to be maintained.

### Drawing

The wxWidgets has a concept of a "Device Context" called `wxDC` through which `wxFrame` objects can "Draw".  For CalChart, drawing is controlled via the `CalChartView` (and `AnimationView`) object.  The general flow is that when a redraw event needs to occur, the `CalChartView` will access the information on what to draw from the `CalChartDoc` (the Marcher position, dot type, direction, path) and use the `CalChartConfiguration` to determine the draw parameters and call the appropriate draw functions.


### CalChartConfiguration

`CalChartConfiguration` interfaces with the system config for parameters that can tweak behavior and acts as a "cache" for the values.  For example, different colors or widths of texts that the user can manipulate for their taste preferences are stored in CalChartConfiguration.  Also, values that should be persist between executions of the CalChart program are stored in CalChartConfiguration.
On `Get`, it reads the values from system config, and caches a local copy.
On `Set` (or `Clear`), it updates it's cache, and puts the command into a write-queue.
The write-queue needs to be explicitly flushed or the values will be lost.
To use a config value, first get the Global config, and then `Get_` the value from it.  For example:

```
auto save_interval = CalChartConfiguration::GetGlobalConfig().Get_AutosaveInterval();
```

To add a new config value, add `DECLARE_CONFIGURATION_FUNCTIONS` in the class declaration of the right type.  This will make the `Get_`, `Set_` and `Clear_` functions available.  Then in the implementation file, declare `IMPLEMENT_CONFIGURATION_FUNCTIONS` with the default.


### CalChartPreferences

`CalChartPreferences` is the Dialog that is used to interact and manipulate values of `CalChartConfiguration`.  `CalChartPreferences` is a `wxNotebook` of different Dialogs that visualize the values in `CalChartConfiguration`, and provide controls for changing their values, as well as a way to visualize what that change would produce.  Because we need a way to manipulate the values without affecting the current values, the approach is to make a local copy of the current Global `CalChartConfiguration` and then manipulate that.  This is the reason for a "write-queue" in the `CalChartPreferences`; we can manipulate a copy of the Configuration to see the effect without affecting the current settings.

So why use this copy/manipulate/assign paradigm than the Undo/Do approach used in other places of CalChart?  Because we don't want to "pollute" the undo stack with modifications of the Configuration.  It would be surprising that Undo would change the color of the background in calchart.


# Style guide

We try to follow some program and C++ best practices.  A great place to read this is
https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines

Try to start each header file with `#pragma once`, similar to the `#!` at the top of a unix script file.






