# Extensions for Safari

## jazz-midi

Safari v.11 and below supported NPAPI and did not require the extension.

#### Safari v.12+

For those without a developer certificate,
Safari requires quite a ceremonial dance *each time* you need to reload the extension:

- Go to `Safari`->`Preferences`->`Extensions` and uninstall the previous version of the extension
- Quit Safari completely (with `⌘-Q`)
- Restart Safari
- Check the `Develope`->`Allow Unsigned Extensions` menu item
- Run the extension app (can do it from Xcode)
- In `Safari`->`Preferences`->`Extensions` click the checkbox to enable the extension