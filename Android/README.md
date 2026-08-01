# Bot or Fold Android

This directory owns the Android application packaging for Holdem. EUI-NEO's Android
branch supplies SDL2/Vulkan runtime integration, while the application ID, assets,
resources, launcher icon, and native app source are owned by this repository.

## Fixed toolchain

- Android SDK Platform 36
- Android NDK `27.0.12077973`
- CMake `3.22.1`
- SDL2 `2.32.10`
- ABI `arm64-v8a`
- Minimum Android API 26
- Vulkan-capable phone

The EUI-NEO submodule must be checked out at Android commit
`36148c066e8eb5e997b4bf80910c4a38afe8f1b5`.

## Build

Install Android Studio with the toolchain above, then run from the repository root:

```powershell
.\Android\scripts\build-apk.ps1
```

The debug APK is written to:

```text
Android/app/build/outputs/apk/debug/app-debug.apk
```

The Android target builds the real Holdem setup page, game core, and table GUI. The
setup page uses a fixed portrait orientation; starting a game switches to a fixed
landscape orientation, and returning to table setup restores portrait orientation.
The activity also closes the soft keyboard before changing orientation so the IME
and SDL surface do not compete to resize the drawable during the transition. Holdem
overrides SDL's Android orientation hook so its resizable native window cannot reset
the activity to `fullUser`. The manifest identifies the app as a game so Android 16
applies its documented game exception for orientation restrictions.

The Android build compiles a generated copy of EUI-NEO with compositor-managed
Vulkan rotation. The pinned EUI backend advertises `currentTransform` without doing
the matching projection, viewport, and scissor pre-rotation; delegating rotation to
Android prevents the rendered surface and SDL input coordinates from diverging.
