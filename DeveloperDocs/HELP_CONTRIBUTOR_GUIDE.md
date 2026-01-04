# CalChart Help System — Quick Start for Contributors

## Quick Links
- **Full Design Document:** [HelpSystemArchitecture.md](HelpSystemArchitecture.md)
- **Markdown Help Files:** [docs/md/](../docs/md/)

## TL;DR — Add a New Help Page in 5 Minutes

### 1. Create Your File
Create a new `.md` file in the appropriate location:
```bash
# In docs/md/reference/ for reference topics
touch docs/md/reference/my-feature.md

# Or in docs/md/tutorials/ for tutorials
touch docs/md/tutorials/my-tutorial.md
```

### 2. Add Frontmatter & Content
```markdown
---
title: My Feature Guide
keywords: feature, help, example
related:
  - reference/overview
---

# My Feature Guide

## Overview
Explain what this feature does.

## How to Use
Step-by-step instructions...

## See Also
- [Related Topic](reference/other.md)
- [Another Topic](../introduction.md)
```

### 3. Link It From index.md
Edit [docs/md/index.md](../docs/md/index.md) and add your page to the appropriate section:
```markdown
### Reference
- [My Feature Guide](reference/my-feature.md) — Description of feature
```

### 4. Test It
```bash
# Build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Run and open Help (Ctrl+H)
./build/src/CalChart
```

## Markdown Format Guide

### Basic Formatting
```markdown
# Main Heading
## Subheading
### Sub-subheading

**Bold text**
*Italic text*
`inline code`

- Bullet point 1
- Bullet point 2
- Bullet point 3

[Link text](relative/path/to/file.md)
[External](https://example.com)

### Code Block
```code
some_code_here()
```
```

### Linking to Other Topics
```markdown
# Use relative paths
[Learn More](../reference/overview.md)
[Another Topic](./sibling.md)
[Internal Link](#section-heading)

# The system will find these automatically
[Basics](../reference/basics.md)
[Introduction](../../introduction.md)
```

### Frontmatter (YAML Metadata)
```yaml
---
title: Page Title          # Required: Human-readable title
keywords: word1, word2     # Optional: Comma-separated search keywords
related:                   # Optional: Related topic IDs
  - reference/basics
  - tutorials/first-show
---
```

## Important Notes

### Content Guidelines
- **Keep it simple** — Use clear language, avoid jargon when possible
- **Include examples** — Show how to actually use the feature
- **Be accurate** — Test what you document before submitting
- **Be current** — Help documentation should reflect the latest version
- **Link contextually** — Connect to related topics naturally

### Markdown Limitations
The help system uses a simple Markdown converter that supports:
- ✓ Headers, bold, italic, code
- ✓ Bullet lists
- ✓ Code blocks
- ✓ Links (internal and external)
- ✗ Tables (not supported yet)
- ✗ Nested lists (use simple lists only)
- ✗ HTML escaping (avoid raw HTML)

### File Organization
- Use `docs/md/reference/` for conceptual/reference documentation
- Use `docs/md/tutorials/` for step-by-step guides
- Top-level files (`docs/md/*.md`) are for main sections
- Use `docs/md/_resources/` for images and other assets

## Common Tasks

### Add a Tutorial
1. Create `docs/md/tutorials/my-tutorial.md`
2. Start with an overview and learning objectives
3. Include step-by-step instructions with examples
4. End with "Next Steps" or "See Also"
5. Link from `docs/md/index.md` under "Tutorials"

### Add Reference Documentation
1. Create `docs/md/reference/topic-name.md`
2. Explain the concept clearly
3. Include examples or use cases
4. Link to related topics
5. Link from `docs/md/reference/` index or parent topic

### Update Existing Documentation
1. Edit the `.md` file directly in `docs/md/`
2. Keep the frontmatter intact
3. Update content accurately
4. Test with local build
5. Submit PR with clear description of changes

### Link Between Topics
Use relative paths, they're automatically discovered:
```markdown
[See Also](../reference/other-page.md)
[Tutorial](../../tutorials/example.md)
[Home](../index.md)
```

## Get Help

- **Questions about help system?** See [HelpSystemArchitecture.md](HelpSystemArchitecture.md)
- **Progress & what's being worked on?** See [HELP_MIGRATION_CHECKLIST.md](HELP_MIGRATION_CHECKLIST.md)
- **Want to contribute?** Open a PR! We welcome documentation improvements
- **Found an error?** Please report it!

---

**Happy documenting!** 📚

Every improvement to the help system makes CalChart more accessible to new users.
