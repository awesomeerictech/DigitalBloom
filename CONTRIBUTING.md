# Contributing to DigitalBloom (Community Edition)

Thank you for your interest in contributing to DigitalBloom. This guide explains how to contribute effectively to a repository that distributes compiled binaries (Android & Windows) and supports documentation, plugins, integrations, and pilot engagements.

**Important:** This repository currently publishes **binaries only** (Android APKs and Windows executables). The core source code is not publicly available in this repo. If you require source access for evaluation or commercial purposes, please contact us (see "Requesting Source / Commercial Licensing" below).

---

## Ways to contribute

1. **Report issues**  
   - Use the repository *Issues* to report bugs or suggest improvements.
   - Provide clear, reproducible steps, platform (Android/Windows), binary version, and screenshots or logs when possible.
   - Use labels: `bug`, `enhancement`, `support`, `docs`.

2. **Improve documentation**  
   - Documentation helps adoption. We welcome:
     - Quickstart guides
     - Deployment notes for Raspberry Pi/VPS/cloud
     - Step-by-step hotspot/offline setup guides
     - Translations (submit as PRs or open issues)
   - Put docs in `/docs` or submit a pull request against the `main` branch.

3. **Create plugin examples / integration guides**  
   - DigitalBloom supports a plugin model. If you have plugin code or a plugin concept (for an external integration or analytics), submit it under `/plugins/examples` as source or pseudo-code and include tests or usage instructions.
   - If you submit code, use the Developer Certificate of Origin (DCO) sign-off (see below).

4. **Share case studies / testimonials**  
   - Real-world examples help us grow. Share success stories, short videos, stats, or customer testimonials by opening an issue or PR with content and media in `/docs/case-studies/`.

5. **Sponsorships & pilots**  
   - If you or your organisation prefers to fund development, we accept paid pilots, sponsorships, and commercial engagements. Contact us directly: stratetacticallimited@gmail.com

---

## How to propose a change (PR workflow)

- Fork the repository and create a branch with a descriptive name (e.g., `docs/hotspot-setup`, `plugin/mpesa-integration`).
- Make changes and include tests or examples where relevant.
- Sign your commit with a DCO line (see below).
- Submit a Pull Request, describing the change, test steps, and impact.
- Maintain one logical change per PR.

**Note:** For binary or compiled artifacts we publish to releases, PRs should not attempt to replace or re-upload signed binaries without maintainers’ approval.

---

## DCO (Developer Certificate of Origin)

We use a lightweight sign-off to track contributor consent.

Add the following to your commit message when submitting code/documentation:

