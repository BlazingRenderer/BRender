# TRM documentation tools

Scripts used to lift the function and type documentation out of the BRender
Technical Reference Manual (`resources/`) and apply it inline to the headers.

The manual's embedded fonts carry no `ToUnicode` map, so `pdftotext` renders the
prose as punctuation soup (function names and code survive, because they are in
Courier).  The pipeline therefore renders each page to an image and has a
vision-capable model transcribe the reference entries verbatim into a fixed
Markdown template; everything after that is deterministic.

## Input

Transcripts live in `contrib/trm/extract/`, one file per reading window
(`w1-*.md` … `w4b-*.md`).  Each entry is a `## Name` section followed by fields
(`description:`, `arguments:`, … for functions; a `members:` list for types).
A wave-4 member is

    ## br_actor

    page: 76
    struct: br_actor
    members:
    - parent — A pointer to the actor's parent actor …
      - BR_…
      ```c
      …
      ```

Blocks in a member body are blank-line separated.  A block that is fenced
(```` ``` ````), a list (`- ` or `N. `) or a Markdown table is kept structured;
anything else is prose.  A fenced block with a `c` info string becomes
`\code{.c}`, any other fenced block becomes `\verbatim`.

The type-level descriptions (each type's "The Structure" prose) live in
`struct-*.md`: one `## <type>` section of prose per type, with the same block
conventions.

## Flow

    dump.py     transcripts -> /tmp/apply/<header>.md    (declaration + fields)
    gen.py      /tmp/apply  -> /tmp/gen/<header>.blocks  (Doxygen blocks)
    apply.py    insert the blocks above their declarations
    sync.py     replace an existing block above a declaration
    members.py  type members -> doc blocks (wave 4)
    structs.py  type descriptions -> doc blocks above the typedefs
    cscan.py / cpurge.py     find and drop .c comments the docs supersede
    plan.py / route.py       counting / reconnaissance

`apply.py` and `sync.py` are deliberately different operations: `apply` inserts
where no doc block exists, `sync` replaces one that does.  Test both, on a path
that has an existing block and one that does not.

## Gates

Never commit a doc change without building and re-running Doxygen:

    make -C cmake-build-32 gltfview -j16
    nix run nixpkgs#doxygen -- Doxyfile        # WARN_IF_DOC_ERROR is on

Doxygen is clean on most runs; when it warns, it is right.  The build catches
data loss in the comments.  Neither catches a collapsed list or a flattened code
block — render the HTML and read it (`doc/html/`).

## Exceptions

`br_pixelmap`'s members live in `BR_PIXELMAP_MEMBERS_PREFIXED()`, parametrised by
a prefix, rather than in a struct body, so `members.py` leaves it out; it is
documented by hand.

The manual's `br_bounds` is this tree's `br_bounds2`/`br_bounds3`/`br_bounds4`;
`members.py` and `structs.py` carry that mapping in `TARGETS`.  `structs.py`
keeps the code-specific notes above `br_euler` and `br_model` (its `KEEP` set)
and inserts the description below them.

`BrPixelmapDirtyRectangleFill` and the conversion macros documented in
`resources/trm-review.md` are not functions and are out of scope here.
