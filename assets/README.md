# Game files

Put the files of your own copy of **Guitar Hero: Aerosmith (Xbox 360)** here, so that this file exists:

```
assets/default.xex
```

The whole disc content goes in this folder. Extract it from your disc or ISO with
[extract-xiso](https://github.com/XboxDev/extract-xiso/releases).

The build needs `default.xex` (the code is recompiled from it); the game needs everything else.
The expected executable has CRC32 `529115C6` (10,760,192 bytes, title ID `41560819`); `GHAerosmithRecomp.exe`
warns at startup if yours is different.

Nothing in this folder is tracked by git.
