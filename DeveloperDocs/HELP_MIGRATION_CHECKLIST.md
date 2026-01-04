# Help System Content Migration Checklist

This file tracks the conversion of legacy HTML help files to modern Markdown format.

## Migration Status

### Phase 0: Foundation (COMPLETE ✓)
Documentation mirror original CalChart in the new system
- [ ] `docs/md/index.md` — Help index/contents


### Phase 1: Basics (IN PROGRESS)
High-level documentation and structure:

- [ ] `docs/md/index.md` — Help index/contents (NEW)
- [ ] `docs/md/introduction.md` — Getting started (NEW)
- [ ] `docs/md/installation-setup.md` — Installation & setup (NEW)
- [ ] `docs/md/reference/overview.md` — Overview/concepts (NEW)
- [ ] `docs/md/reference/basics.md` — Basic concepts (NEW)

### Phase 2: Core Reference Content
Main reference documentation. Priority order based on usage frequency.

#### High Priority Reference Topics
- [ ] `reference/user-interface.md`
  - Source: `docs/charthlp_overview.html`, `docs/charthlp.html`
  - Status: —
  - Covers: Main UI layout, panels, toolbars, menus

- [ ] `reference/movement.md`
  - Source: `docs/charthlp_movement.html`
  - Status: —
  - Covers: Moving marchers, animation basics, paths

- [ ] `reference/selecting-points.md`
  - Source: `docs/charthlp_selectingpoints.html`
  - Status: —
  - Covers: Selecting marchers, tools for selection

- [ ] `reference/animation.md`
  - Source: `docs/charthlp_animcont.html`, `docs/charthlp_editcont.html`
  - Status: —
  - Covers: Animation commands, continuity editing, transitions

#### Medium Priority Reference Topics
- [ ] `reference/keyboard-shortcuts.md`
  - Source: NEW (currently not well documented)
  - Status: —
  - Covers: All keyboard shortcuts and hotkeys

- [ ] `reference/field-properties.md`
  - Source: `docs/charthlp_fieldproperties.html`
  - Status: —
  - Covers: Field dimensions, grid, configuration

- [ ] `reference/formation-tools.md`
  - Source: `docs/charthlp_shapes.html` (possibly) 
  - Status: —
  - Covers: Creating formations, shape tools, patterns

- [ ] `reference/continuity-output.md`
  - Source: `docs/charthlp_printcont.html`
  - Status: —
  - Covers: Printing continuity, formatting options

- [ ] `reference/advanced-techniques.md`
  - Source: `docs/charthlp_magic.html`, `docs/charthlp_geniusmove.html`
  - Status: —
  - Covers: Reference groups, advanced moves, tricks

#### Lower Priority Reference Topics
- [ ] `reference/exporting.md`
  - Source: NEW
  - Status: —
  - Covers: Export formats, PostScript output

- [ ] `reference/configuration.md`
  - Source: NEW (scattered in preferences docs)
  - Status: —
  - Covers: Preferences, settings, customization

### Phase 2: Tutorials (IN PROGRESS)
Step-by-step guides for common tasks.

- [ ] `tutorials/creating-first-show.md`
  - Source: Synthesized from `charthlp_newshow.html`, `charthlp_basics.html`
  - Status: —
  - Covers: Creating a new show from scratch

- [ ] `tutorials/animating-formations.md`
  - Source: Synthesized from `charthlp_movement.html`, `charthlp_animcont.html`
  - Status: —
  - Covers: Creating animated transitions between formations

- [ ] `tutorials/working-with-continuity.md`
  - Source: `docs/charthlp_editcont.html`
  - Status: —
  - Covers: Editing continuity, using continuity editor

- [ ] `tutorials/importing-shows.md`
  - Source: NEW (feature exists, not well documented)
  - Status: —
  - Covers: Importing show files, compatibility

### Phase 2: Standalone Pages (IN PROGRESS)

- [ ] `faq.md`
  - Source: NEW (synthesized from issues and support requests)
  - Status: —
  - Covers: Frequently asked questions

- [ ] `troubleshooting.md`
  - Source: `docs/charthlp_bugs.html` (partially)
  - Status: —
  - Covers: Common issues and solutions, error messages

- [ ] `copyright.md`
  - Source: `docs/charthlp_copyright.html`
  - Status: —
  - Covers: Copyright notice, license

- [ ] `acknowledgments.md`
  - Source: `docs/charthlp_acknowledge.html`
  - Status: —
  - Covers: Credits, contributors, acknowledgments

### Phase 3: Specialized/Advanced Topics (FUTURE)
Less commonly used features and advanced topics.

#### Windows/Platform-Specific
- [ ] `platform/windows-setup.md`
  - Source: `docs/charthlp_wininstall.html`, `docs/charthlp_winprinter.html`
  - Status: —
  - Notes: Outdated, may need significant updating

- [ ] `platform/macos-setup.md`
  - Source: `docs/charthlp_macosxinstall.html`, `docs/charthlp_macosxprinter.html`
  - Status: —
  - Notes: Outdated, may need updating for current macOS

#### Animation/Technical
- [ ] `reference/animation-commands.md`
  - Source: `docs/charthlp_animcont.html` (specialized)
  - Status: —
  - Covers: Detailed animation command syntax

- [ ] `reference/continuity-syntax.md`
  - Source: `docs/charthlp.con` (if applicable)
  - Status: —
  - Covers: .shw file format, continuity syntax

#### Advanced Features
- [ ] `reference/marching-formations.md`
  - Source: Various `charthlp_*.html` files
  - Status: —
  - Covers: Common marching drill formations

- [ ] `reference/mathematical-operations.md`
  - Source: `docs/charthlp_refer.html`
  - Status: —
  - Covers: Rotation, scaling, mathematical transformations

### Phase 4: Legacy Files to Remove (FUTURE)

Once all content is migrated, these files should be removed:

```
docs/charthlp.con
docs/charthlp.ref
docs/charthlp.tex
docs/charthlp.hhc          # Windows CHM index
docs/charthlp.hhk          # Windows CHM keywords
docs/charthlp.hhp          # Windows CHM project file
docs/charthlp_*.html       # All legacy HTML files (100+ files)
docs/*.gif                 # Navigation graphics (no longer needed)
docs/*.tex                 # LaTeX source files
docs/*.ini                 # Old tool config files
```

## How to Mark Progress

When working on a conversion:

1. **Starting:** Change `- [ ]` to `- [x] IN PROGRESS`
2. **In Progress:** Update Status field with % complete and any notes
3. **Complete:** Change to `- [x] DONE ✓`
4. **For documentation, also:**
   - Add frontmatter metadata (title, keywords, related)
   - Link from relevant pages
   - Update `index.md` if applicable
   - Test with local build

## Guidelines for Conversions

### Approach
1. Read the original HTML file to understand structure and content
2. Create new Markdown file with appropriate frontmatter
3. Reorganize content for clarity (HTML may have poor structure)
4. Update outdated information (many files are 10+ years old!)
5. Add missing information or cross-references
6. Test with local build of CalChart

### Documentation Age Notes

Many HTML help files haven't been updated in 10+ years. When converting:
- **Check version-specific information** — May reference old CalChart versions
- **Verify UI locations** — Menu locations, dialog layouts may have changed
- **Update tool names** — Some tools may have been renamed or consolidated
- **Add missing features** — Newer features may not be documented
- **Remove obsolete info** — Old workarounds or deprecated features
- **Improve clarity** — Many docs could benefit from reorganization

### Example Areas That Are Very Outdated
- Installation guides (many Windows-specific, probably not relevant)
- Printing instructions (has changed significantly)
- Export formats (may have changed)
- UI descriptions (layout has evolved)
- Platform-specific instructions

## Helpful Resources

- **[Help Contributor Guide](HELP_CONTRIBUTOR_GUIDE.md)** — Quick reference for adding content
- **[Help System Architecture](HelpSystemArchitecture.md)** — Full design and conventions
- **[Help System Implementation](HELP_SYSTEM_IMPLEMENTATION.md)** — Technical details
- **[Markdown Files Directory](md/)** — Existing converted help files

## Statistics

- **Total HTML files in legacy system:** ~100+
- **Files converted (Phase 1):** 5
- **Files converted (Phase 2):** 0
- **Overall % complete:** ~5%
- **Est. pages remaining:** 95+

## Priority Ranking Rationale

### High Priority (Most Used)
1. User interface guide — Users need this first
2. Movement/animation — Core feature
3. Selection tools — Fundamental workflow
4. Animation system — Essential for creating shows

### Medium Priority (Important But Less Urgent)
1. Keyboard shortcuts — Reference material
2. Field properties — Configuration
3. Formation tools — Common workflow
4. Continuity output — Final product
5. Advanced techniques — For experienced users

### Lower Priority (Specialized)
1. Export/import functionality
2. Installation (generally stable now)
3. Platform-specific guides (outdated and platform-specific)
4. FAQ/troubleshooting (built as needed)

---

## Notes

- **Last Updated:** 2025-01-04
- **Target Completion:** Phase 2 by end of Q1 2025, Full migration by end of 2025
- **Volunteer Contributions:** Welcome! Please comment on GitHub issues to coordinate
