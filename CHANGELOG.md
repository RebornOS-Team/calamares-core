# CHANGELOG

## Calamares Core `v3.3.9_002`

### For Users

- Removed the `crc32-intel` module, as it is deprecated in favor of `crc32c`

### For Developers

NA

## Calamares Core `v3.3.9_001`

### For Users

- Update the installer from upstream, bringing new features and bug-fixes.

### For Developers

NA

## Calamares Core `v3.3.8_002`

### For Users

- Make the partition label color gray from black to improve readability.
- Update the installer from upstream, bringing new features and bug-fixes.

### For Developers

NA

## Calamares Core `v3.3.5_001`

### For Users

- Update the installer from upstream, bringing new features and bug-fixes.

### For Developers

- Build against Qt6, update dependencies, and fix bugs resulting from the transition.

## Calamares Core `v3.3.2_001`

### For Users

- Update the installer from upstream, bringing new features and bug-fixes.

### For Developers

- Prune build dependencies

## Calamares Core `v3.3.0_alpha006_001`

### For Users

- Update the installer from upstream, bringing new features and bug-fixes.

### For Developers

- Re-enable the use of `pybind11` instead of boost.
- Remove dependency on `boost`.

## Calamares Core `v3.3.0_alpha005_005`

### For Users

- Fix hanging while mounting happens in the installer

### For Developers

- Disable the use of `pybind11`. It currently prevents the python modules from running.

## Calamares Core `v3.3.0_alpha3_010`

### For Users

- New features and bug fixes from upstream incorporated.
- Better pacman progress reporting

### For Developers

- CONTRIBUTING.md has a release checklist.

## Calamares Core `v3.3.0_alpha3_8`

### For Users

- Installer text colours, and image colours all modified to suit the dark mode.
- New features and bug fixes from upstream incorporated.

### For Developers

- Launch script modified to send more environment information, and to remove `dbus-launch`.
- Enabled launching processes from the `welcomeq` module.
- Renamed the RebornOS GitHub organization from `RebornOS-Developers` to `RebornOS-Team`.
- New features and bug fixes from upstream incorporated.
- Updated and faster script to pull upstream changes into a separate `_calamares` branch.
- The `README.md` documentation is updated.

## Calamares Core `v3.3.0_alpha2_1` -> `v3.3.0_alpha3_5` ChangeLog

### For Users

- In the *Advanced* page (the `netinstall` module) all duplicate package names now share the same selection state. This means that if you see the same package under multiple categories, all duplicates will update when you either click on those packages directly or when you indirectly select/deselect them by clicking on a *group*

- The *Advanced* page (the `netinstall` module) now has a *Reset to Defaults* functionality

- `adobe-source-han-sans-otc-fonts` and `adobe-source-han-serif-otc-fonts` added to properly display Chinese, Japanese, and Korean fonts in the installer

- Drives mounted/auto-mounted from the ISO are now automatically unmounted before the installer launch, allowing the partition page in the installer to provide all possible options for those drives.


### For Developers

- GitHub CI for stable releases complete with GitHub-built Arch Linux packages, ChangeLog fetched from CHANGELOG.md, and MD5 CheckSums

- GitHub CI for unstable pre-releases complete with GitHub-built Arch Linux packages, ChangeLog fetched from commit history, and MD5 CheckSums

- Revamped and updated documentation
  
- Revamped and simplified build scripts for three packaging modes - `local`, `git`, and `stable`
  
- Updated *PKGBUILDs* to prevent the annoying replacement offers for the installer packages during system updates