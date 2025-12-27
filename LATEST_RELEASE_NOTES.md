## Release notes for 3.8.4

### New Features

* **Bug Reporting**: Users can now report bugs directly from CalChart
  * Access via **Help → Report a Bug** or press **Ctrl+Shift+B**
  * Automatic system information collection (OS, version, CalChart build info)
  * Optional show information and custom details
  * Privacy controls: opt-in checkboxes for each data category
  * Automatic submission to GitHub issues when configured with a GitHub token
  * Clipboard fallback for users without GitHub token setup
  * Full error reporting with helpful messages
  * See [docs/BugReporting.md](docs/BugReporting.md) for setup instructions

Bugs addressed in this release:

Other changes:

* [#635](../../issues/635) Reduce usage of wxString
* [#648](../../issues/648) Make CI go faster
* [#689](../../issues/689) Update wxWidgets to 3.3.1
* [#718](../../issues/718) Bug filing system
* [#729](../../issues/729) Reduce the number of warnings from windows builds
* [#731](../../issues/731) Move tests into the right places
* [#732](../../issues/732) remove wxUI::Custom
* [#735](../../issues/735) Update the CI to have the release or debug in the name
* [#737](../../issues/737) Print the version of wxWidgets used
* [#751](../../issues/751) Add logs to bug report

