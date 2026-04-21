# Contributing

Join us on the Elasto Mania Discord https://discord.gg/j5WMFC6 in #elma-developers!

## Building

See [docs/BUILDING.md](./docs/BUILDING.md)

## Commits

### Every commit must be atomic

Each commit should make sense as a single logical change. Use your best judgement - a small helper function introduced alongside its only caller is fine in one commit, but unrelated changes should be split up.

### Every commit must build and pass formatting

Each commit in a branch must independently:

1. **Compile** — `meson compile -C build` succeeds
2. **Pass clang-format** — `ninja -C build clang-format-check` reports no changes

Never leave formatting fixes to a separate commit. Run `ninja -C build clang-format` before staging your changes.

If you're rebasing a branch with many commits, verify retroactively with:

```bash
git rebase --exec 'ninja -C build clang-format-check' main
```

### Commit message format

Use [Conventional Commits](https://www.conventionalcommits.org/):

```
type(scope): description
```

Keep the subject line under 72 characters.

**Types:** `feat`, `fix`, `refactor`, `chore`

**Scope:** the subsystem or file being changed (e.g., `menu_nav`, `physics`, `editplay`)

**Examples:**

```
feat(menu_play): add "Level replays" to post-game menu
fix(physics): prevent wheel clipping through thin polygons
refactor(editplay): extract toolbar rendering into helper
chore(meson): bump SDL2 to 2.32.8
```

The subject line should be self-explanatory. Use the body to explain _why_, not _what_, when the motivation isn't obvious from the diff.

## Branches

### Fixing mistakes on a feature branch

If you need to fix something introduced earlier on the same branch (not yet merged to `main`), don't create a standalone fix commit. Instead, amend the original commit so the branch history reads as if the code was correct from the start:

```bash
git commit --fixup=<target-hash>
git rebase --autosquash main
```

Standalone fix commits are only appropriate for issues that already exist on `main`.

### Commits on main are immutable

Once commits are merged to `main`, they are never rebased, amended, or squashed. If something needs changing, create a new commit on a new branch.

## Code style

- **Formatting:** clang-format enforced (LLVM-based, 4-space indent, 100-column limit, no tabs, left-aligned pointers, braces always inserted)
- **Constants:** `SCREAMING_SNAKE_CASE`
- **Global variables:** `PascalCase`
- **Functions / local variables:** `snake_case`
- **Struct/class member variables:** trailing underscore (`name_`, `screen_width_`)
- **Filenames:** new files use lowercase (`menu_play.cpp`). Uppercase files (`EDITTOLT.CPP`) are legacy and will be renamed over time.
