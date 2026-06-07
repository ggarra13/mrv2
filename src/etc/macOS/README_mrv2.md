
To install it, after opening the .dmg file, drag the mrv2 icon to the Applications directory.  If there's already an mrv2 version, we recommend you overwrite it (in case it does not work well for you, you can always revert it.

As the mrv2 application is currently neither code signed nor notarized, you need to do some fearful steps.

With the Finder, open /Applications/Terminal.app

In the Terminal window, run these commands:
  
```
  sudo xattr -cr /Applications/mrv2.app/
```

Enter your password.  Then, right after, run these commands:

```
  sudo codesign --force --sign - /Applications/mrv2.app/Contents/Resources/bin/*
  sudo codesign --force --sign - /Applications/mrv2.app/Contents/Resources/lib/*
  sudo codesign --force --deep --sign - /Applications/mrv2.app

  codesign -vvv --deep /Applications/mrv2.app
```
  
  If the last step prints "satisfies its Designated Requirement", you're gold.

