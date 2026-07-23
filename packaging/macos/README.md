# macOS app bundle

This packaging path builds a self-contained `OpenOMF.app` without relying on Homebrew at runtime. The script copies OpenOMF resources, shaders, controller mappings, and any original game assets from `data/` into the app bundle.

The recommended dependency source is vcpkg manifest mode. If `VCPKG_ROOT` points at a vcpkg checkout, the script passes its toolchain file to CMake. Otherwise CMake falls back to the normal dependency discovery rules.

The macOS-specific vcpkg overlay ports live under `packaging/macos/vcpkg-ports/`. The root `vcpkg-configuration.json` includes that path so vcpkg can find them during manifest builds.

The app icon source image is `packaging/macos/omf2097.png`; the bundle icon is the generated `packaging/macos/openomf.icns`. The packaging script copies `openomf.icns` into the app bundle and references it with `CFBundleIconFile`.

## Build

```sh
VCPKG_ROOT=/path/to/vcpkg \
VCPKG_TARGET_TRIPLET=arm64-osx \
OPENOMF_BUNDLE_ID=com.example.OpenOMF \
OPENOMF_CODESIGN_IDENTITY="Developer ID Application: Your Name (TEAMID)" \
./packaging/macos/build-app.sh
```

The output path defaults to `dist/macos/OpenOMF.app`.

If `OPENOMF_CODESIGN_IDENTITY` is omitted, the script ad-hoc signs the app for local testing.

## Notarization

Developer ID signing is enough to produce signed code, but distribution to other Macs should also notarize and staple the app:

```sh
xcrun notarytool submit dist/macos/OpenOMF.app.zip --keychain-profile PROFILE --wait
xcrun stapler staple dist/macos/OpenOMF.app
```

## GitHub Actions

`.github/workflows/macos-app.yml` builds a zipped `OpenOMF.app` artifact from GitHub Actions.

For ad-hoc signed test artifacts, run the workflow manually and leave `codesign_identity` blank.

For Developer ID signing, add these repository secrets and set `codesign_identity` to the Developer ID Application identity name or SHA-1:

- `MACOS_CERTIFICATE_P12_BASE64`: base64-encoded Developer ID Application `.p12`
- `MACOS_CERTIFICATE_PASSWORD`: password for the `.p12`
- `MACOS_KEYCHAIN_PASSWORD`: temporary CI keychain password

For notarization, enable the workflow's `notarize` input and add:

- `NOTARY_APPLE_ID`: Apple ID email
- `NOTARY_PASSWORD`: app-specific password
- `NOTARY_TEAM_ID`: Apple Developer Team ID
