# Binary-Only Distribution Policy

This repository distributes **compiled binaries only** for the DigitalBloom Community Edition.

The source code for the core platform is **not published in this repository**. This policy is intentional and supports security, controlled distribution, and commercial sustainability while still allowing transparent usage and evaluation.

---

## What is provided here

- Signed **Android APK** builds
- Signed **Windows executables**
- Documentation, guides, and examples
- Plugin specifications and integration references
- Issue tracking and support guidance

No core source code is included in this repository.

---

## Supported platforms

DigitalBloom binaries are provided for:
- Android
- Windows

The platform itself supports deployment across Android, Linux, Windows, macOS, embedded systems (e.g., Raspberry Pi), and cloud VPS environments. Some deployments may require guided setup or commercial engagement.

---

## Release process (maintainers)

Each public release MUST follow this checklist to ensure integrity and trust.

### Release checklist
- [ ] Version number updated and tagged (e.g., `v1.2.0`)
- [ ] Changelog updated with:
  - New features
  - Bug fixes
  - Security notes (if applicable)
- [ ] Binaries built from a clean environment
- [ ] Android APK signed with official release keystore
- [ ] Windows executable signed with code-signing certificate (if available)
- [ ] SHA-256 checksums generated for all binaries
- [ ] Checksums published in the release notes
- [ ] Virus/malware scan completed on binaries
- [ ] Release artifacts uploaded to GitHub Releases
- [ ] Release notes reviewed for accuracy and clarity

---

## Binary signing guidance

### Android (APK)
- Use a dedicated **release keystore**, stored securely and backed up offline.
- Never commit keystore files or passwords to the repository.
- Sign APKs using standard Android signing tools.
- Verify signature before publishing.

### Windows (EXE)
- Use an Authenticode code-signing certificate where possible.
- If a certificate is not available, publish:
  - SHA-256 checksum
  - Build environment details
- Ensure the executable is scanned for malware before release.

---

## Verifying downloads (users)

Users can verify integrity by:
1. Downloading the binary from the official GitHub Releases page.
2. Computing the SHA-256 checksum locally.
3. Comparing it with the checksum listed in the release notes.

Only binaries published via official releases should be trusted.

---

## Source access & commercial licensing

Organisations requiring:
- Source code access
- Custom builds
- White-label distributions
- Enterprise security reviews
- SLA-backed support

may request access under **commercial or NDA terms**.

**Contact:**  
Email: stratetacticallimited@gmail.com

Please include organisation name, intended use, and any compliance requirements.

---

## Security reporting

Security vulnerabilities must be reported privately.  
See `SECURITY.md` for responsible disclosure instructions.

---

## Disclaimer

DigitalBloom binaries are provided **“AS IS”**, without warranty of any kind, in accordance with the Apache License 2.0. Use at your own risk.

---

Thank you for respecting this distribution model and helping keep DigitalBloom reliable and secure.
