# Toolchain risk and sustainability

Elkhart's build chain depends on a toolchain that is no longer maintained
upstream. This is a long-term sustainability concern worth surfacing so
future maintainers (and future-me) can act before the chain breaks.

## What we depend on

- **avr-gcc 4.3.5 / avr-libc 1.6.8.** Required because YAM's oscillator
  DSP miscompiles under newer toolchains. Empirically confirmed: avr-gcc
  5.4 and 8.5 both produce a constant metallic-tone artifact overlaid on
  audio output (verified on hardware 2026-04-30). The miscompile is not
  specific to elkhart — it affects upstream YAM and stock Mutable
  Instruments firmware as well, since all three keep the original
  oscillator algorithms (CZ resonant, PolyBLEP, FM, vowel, `uint24_t`
  phase struct).
- **Debian Squeeze (`debian/eol:squeeze`) container image**, pinned by
  digest:

  ```
  FROM debian/eol@sha256:80ed3d6f933f102b35efaa1a9a7513e5ed9937c5a93c289dd6e2e1db51d6f331
  ```

  Squeeze reached EOL in February 2016. The image is the only thing
  packaging the right avr-gcc + avr-libc together; we build once with
  network access, then every compile runs `--network none --cap-drop=ALL
  --rm`. `apt-get` sources inside the image already point at
  `archive.debian.org`.

## Failure modes

The chain breaks if any of the following happens:

1. **Docker Hub purges the `debian/eol` repository or the pinned digest.**
   Hub's policy on EOL images is unclear; pinned-by-digest is more
   durable than a tag but not permanent.
2. **`archive.debian.org` removes Squeeze packages.** The image already
   has avr-gcc / avr-libc installed, so an existing local image survives;
   but rebuilding from `Dockerfile.squeeze` would fail because the apt
   step needs the archive.
3. **Local Docker storage loss.** If `ambika-squeeze:latest` is purged
   from the dev machine and neither (1) nor (2) can be re-fetched, the
   build chain is dark.

The pinned digest in `Dockerfile.squeeze` mitigates the "wrong image
tag" failure mode but not removal.

## Mitigations to consider

In rough order of effort vs. permanence:

### Cheap, soon

- **Save a local backup of the built image.** `docker save
  ambika-squeeze:latest -o ambika-squeeze.tar` (~200 MB). Stash in
  off-repo backup (the image itself is too large for git).
- **Pin the build packages by SHA.** Capture `dpkg -l` output from
  inside the running image into a checked-in manifest, so we know
  exactly which `.deb` versions to look for if rebuilding from raw
  packages becomes necessary.

### Medium effort

- **Vendor the `.deb` files into a private archive.** Pull the exact
  `gcc-avr_4.3.5-...deb` and `avr-libc_1.6.8-...deb` (plus dependencies)
  from `archive.debian.org` while it still serves them, stash off-repo
  alongside the image tarball, and rewrite `Dockerfile.squeeze` to
  `COPY` + `dpkg -i` instead of `apt-get install`. Removes the dep on
  `archive.debian.org` entirely.
- **Vendor inside the repo.** Same as above but committed under
  `toolchain/vendor/`. ~30 MB of `.deb` files in git history — feasible
  but bloats clones. Probably not worth it unless the off-repo backups
  feel fragile.

### High effort, most permanent

- **Migrate off avr-gcc 4.3.5.** Re-test newer avr-gcc releases against
  YAM's oscillator DSP. The miscompile is in fixed-point arithmetic
  paths; either find the gcc patch that broke it (and patch around it),
  or rewrite the affected DSP in a form that survives modern codegen.
  Could be combined with revisiting YAM's `__attribute__((naked))`
  prologues and `uint24_t` packing, both of which interact poorly with
  modern optimizers.
- **Switch to a maintained toolchain entirely.** llvm-avr is the obvious
  candidate; behaviour against YAM DSP unknown.

## Decision posture

Today (2026-05) the chain works and digests are pinned. The risk is
real but not imminent. The cheapest mitigations (save the image,
pin package versions) cost an afternoon and meaningfully derisk the
two most likely failure modes; consider doing them before the next
release cycle.

The DSP-migration path is a major project and not urgent unless one
of (1)–(3) above actually happens, or unless an unrelated reason
forces a toolchain refresh.
