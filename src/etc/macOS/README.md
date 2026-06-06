
To install it by opening the .dmg file, and dragging the mrv2, vmrv2 and optional hdr icon to the Applications directory.  If there's already an mrv2 version, we recommend you overwrite it.


With the Finder, open /Applications/Terminal.app

In the Terminal window, run these commands:
  
```
  sudo xattr -cr /Applications/vmrv2.app/
```

Enter your password.  Then run these commands:

```
  sudo codesign --force --sign - /Applications/vmrv2.app/Contents/Resources/bin/*
  sudo codesign --force --sign - /Applications/vmrv2.app/Contents/Resources/lib/*
  sudo codesign --force --deep --sign - /Applications/vmrv2.app

  codesign -vvv --deep /Applications/vmrv2.app
```
  
  If it prints satisfies its Designated Requirement, you're gold.

