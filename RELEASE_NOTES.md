
# DigitalBloom – Release Notes

## Version 27.10

**Release Date:** *(2025-10-27)*
**Edition:** Community Edition (Binary Distribution)
**Maintained by:** Stratetactical / STL Limited

---

## Overview

Version **27.10** continues the evolution of DigitalBloom as a **cross-platform marketing and communication platform** focused on QR-driven customer engagement, offline-capable deployments, and modular extensibility.

This release emphasizes **stability, performance improvements, and deployment reliability** across Android and Windows environments.

---

## Included Binaries

This release includes compiled, signed binaries located under:

```
/binaries/
```

### Platforms

* **Android**

  * `android-build-DigitalBloom-release-signed-arm32.apk` for older android phones
  * `android-build-DigitalBloom-release-signed-arm64.apk`  for latest android phones
* **Windows**

  * `DigitalBloom-27.10-win64.exe` for 64 bit windows

Only binaries published in this directory and the official GitHub Release are supported.

---

## What’s New

* Improved QR marketing link generation and handling
* Enhancements to marketing page rendering and responsiveness
* Updates to plugin loading and initialization
* Improved startup reliability on constrained or offline networks
* Documentation updates for deployment and usage

---

## Improvements

* Performance optimisations for embedded and edge deployments
* Reduced memory usage during media-heavy marketing pages
* More predictable behavior in local Wi-Fi / hotspot environments
* Improved logging and diagnostics for troubleshooting

---

## Bug Fixes

* Fixed intermittent QR decoding issues on certain Android devices
* Resolved startup failures on Windows systems with restricted permissions
* Corrected media playback inconsistencies in some edge cases
* Fixed minor UI layout issues on smaller screens

---

## Known Limitations

* Source code is not included in this distribution
* Some advanced plugins and AI modules are available only through commercial engagement
* Very large media files may require additional tuning on low-resource devices

---

## Security Notes

* No known critical security vulnerabilities at the time of release
* Users are encouraged to upgrade to this version if running older builds

Security issues should be reported privately. See `SECURITY.md`.

---

## Upgrade Notes

* Existing deployments can replace previous binaries directly
* Configuration files and metadata archives are backward compatible
* Always back up data before upgrading production deployments

---

## Verification

SHA-256 checksums for all binaries in this release are provided in:

```
checksums.txt
```

Users should verify binary integrity before installation.

---

## Support & Feedback

For:

* Bug reports
* Usage questions
* Commercial pilots or support

Contact:
📧 **[stratetacticallimited@gmail.com](mailto:stratetacticallimited@gmail.com)**

---

## License

DigitalBloom is licensed under the **Apache License, Version 2.0**.
See the `LICENSE` file for details.

---

