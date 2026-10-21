# Contributing

## Commit messages

Commits follow a style close to the Linux kernel's and FFmpeg's, and **not**
Conventional Commits — no `feat:`, `fix:`, `chore:`, `docs:`, `style:`,
`refactor:`, `test:`, `ci:`. The
[FFmpeg developer guide](https://ffmpeg.org/developer.html#toc-Patches_002fCommitting)
is the reference for the parts not written out here.

```
component: short description

Optional body.
```

The prefix names the thing that changed, relative to the repo root, and names the
module rather than the directory holding it: `core/fmt/gltf:` for the glTF format
layer, never `core/fmt:`, which is every format in the engine. Likewise
`core/v1db:`, `drivers/softprim:`, `contrib/editorcam:`. Name the component, not
the set of files it happened to touch — the wiring and callers a change drags
along don't broaden it. Keep the subject under 72 characters and wrap the body at
72.

Each commit is a single logical unit, and each one should stand on its own:
split work across commits when it touches independent subsystems, and prefer a
larger commit to a pair where the first is incomplete. A change that moves pixels
or adds a fixture carries its reference-table and fixture update with it, and a
comment the change makes false is corrected in the commit that breaks it.

### The subject

It completes the sentence "This commit will…", and it names the concrete thing
added, renamed or removed — a symbol, an endpoint, a config key — rather than the
area it lives in or the abstraction it implements: `add a package-level Submit()`,
`split into /livez and /readyz, drop /health`.

### The body

The default is no body. Add one only when a reader with the subject and the diff
would be missing something: a changed contract, a format, something forbidden, why
the obvious alternative is wrong, what a refusal protects against. A load-bearing
reason that would read equally well as a code comment belongs in the code.

Length is earned by reference, not by story. One or two sentences is the default;
a long body is right when it carries what the diff cannot — another system's
behaviour, a layout, a table of cases, the alternatives rejected and why — because
it is then read years later as the documentation for the code, and earns structure
and lists. It is not earned by recounting the change, the fault, or reasoning that
is already in the code. The test for each sentence: would a reader holding the
subject and the diff be missing this? "I found that libarchive's …" fails;
"libarchive's three string APIs all require a locale" does not.

**Nothing that can go stale.** No commit hashes, no issue numbers, no counts of
files, fixtures or scenes, no bare measurements (`5d23da96, 320x240 8bpp`). A
rebase or a squash invalidates the first two, and the tree grows out from under
the rest. Say "the branch this was ported from" and cite the scene or the spine
instead.

A body does not restate the subject, and it does not carry the defect that was
found, how it was found, or what a later commit does — that is an issue comment.

**A list says what a set is *for*, not what is in it.** Several distinct changes —
new knobs, behaviours — belong in a bulleted list under the subject; the inventory
the diff already carries (fixtures, files, map channels) does not, and neither
does a count of code shape ("seven parallel arrays").

### Examples

```
gitignore: ignore /build-*/
```

```
core/v1db: fix RegenerateVertexNormals to respect smoothing groups
```

```
contrib/editorcam: fix camera offset after pan mode, add orbit mode

- Pan mode now applies translations to actor (world position) rather
  than camera (orientation), fixing offset after switching modes
- Add orbit mode (Alt+LMB drag) with azimuth/elevation rotation
```

## Sending a change

- Rebase against current `master` before asking for a review.
- Run `clang-format` with the tree's `.clang-format` before you send it.
- Merge commits are not accepted, except at maintainer discretion — octopus merges, or a case where preserving branch history has clear
  value. When in doubt, rebase.
