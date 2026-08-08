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

If Gradle or SDL2 must be downloaded through a local HTTP proxy:

```powershell
.\Android\scripts\build-apk.ps1 -Proxy http://127.0.0.1:9090
```

The debug APK is written to:

```text
Android/app/build/outputs/apk/debug/app-debug.apk
```

The Android target builds the real Holdem setup page, game core, and table GUI. The
setup page follows the user's device orientation and switches between its
single-column and two-column layouts using the actual SDL viewport. Starting a game
switches to a fixed landscape orientation; returning to table setup restores
`fullUser` orientation.
The activity also closes the soft keyboard before changing orientation so the IME
and SDL surface do not compete to resize the drawable during the transition. Holdem
overrides SDL's Android orientation hook so its resizable native window cannot
replace this page-specific policy. The manifest identifies the app as a game so
Android 16 applies its documented game exception for orientation restrictions.

The Android build compiles a generated copy of EUI-NEO with compositor-managed
Vulkan rotation. The pinned EUI backend advertises `currentTransform` without doing
the matching projection, viewport, and scissor pre-rotation; delegating rotation to
Android prevents the rendered surface and SDL input coordinates from diverging.
During a viewport change, the generated SDL loop also keeps requesting full frames
until the drawable size is stable, allowing Vulkan swapchain recreation and the
responsive setup form to settle without requiring a touch event.

The two hand-ranking tables remain external files. `resources/*.bin` is copied into
the APK assets, and the activity copies the asset tree into its internal files
directory before SDL starts. The shared resource locator then loads the same
`resources/card5_dic_zipped.bin` or `resources/card5_dic_zipped_shortdeck.bin`
path on Android and desktop; no generated ranker C++ source is embedded in the APK.
