# GitHub Token Setup for Bug Report Submission

## Overview

CalChart can submit bug reports directly to the GitHub issue tracker if configured with a GitHub Personal Access Token (PAT). Without a token, bug reports are formatted as markdown and copied to the clipboard for manual submission.

## Submitting a Bug Report

### In-Application Flow

1. **Open the Bug Report Dialog**
   - From the menu: **Help → Report a Bug...**
   - Or press **Ctrl+Shift+B**

2. **Fill Out the Form**
   - **Title*** - Brief summary of the bug
   - **Description*** - Detailed explanation of what happened
   - **Steps to Reproduce** - How to trigger the bug
   - **Email** (optional) - For GitHub to contact you with updates

3. **Choose What Information to Share**
   - **Include system information** - OS, architecture, CalChart version (checked by default)
   - **Include show information** - File structure details if a show is open (checked by default if show is loaded)
   - **Preview** - See exactly what will be sent before submission

4. **Submit the Report**
   - Click **Submit**
   - **If you don't have a GitHub token set up:**
     - A dialog will appear with two options:
       - **"Enter token"** - You can paste your GitHub Personal Access Token directly
         - Follow the in-dialog instructions to create a simple classic token (just requires `public_repo` scope)
         - Token is used only for this session
       - **"Open in browser"** - Opens GitHub with a pre-filled issue form containing your bug report
         - Review and submit directly on GitHub's website
         - Easiest option if you don't want to set up a token
     - Choose whichever option works for you
   - **If you have a GitHub token set up:**
     - Your bug report will be submitted automatically
     - You'll see the GitHub issue link on success

## Setting Up GitHub Authentication

### Option 1: Using Environment Variable (Recommended)

Set the `CALCHART_GITHUB_TOKEN` environment variable before launching CalChart:

#### macOS / Linux
```bash
export CALCHART_GITHUB_TOKEN=your_github_pat_here
./CalChart
```

#### macOS (in .zshrc or .bash_profile)
Add this line to your shell configuration:
```bash
export CALCHART_GITHUB_TOKEN=your_github_pat_here
```

#### Windows (PowerShell)
```powershell
$env:CALCHART_GITHUB_TOKEN = "your_github_pat_here"
& ".\CalChart.exe"
```

#### Windows (Command Prompt)
```cmd
set CALCHART_GITHUB_TOKEN=your_github_pat_here
CalChart.exe
```

### Creating a GitHub Personal Access Token

#### Recommended: Classic Personal Access Token (Simple & Reliable)

For CalChart bug reporting, a simple classic token with `public_repo` scope is the easiest and most reliable option:

1. Go to [GitHub Settings - Personal Access Tokens](https://github.com/settings/tokens)
2. Click **Generate new token (classic)**
3. **Token name**: "CalChart FileABug Token"
4. **Expiration**: 90 days (recommended)
5. Under **Scopes**, select **only**: `public_repo`
   - This grants the minimum needed to create public issues
   - Do NOT select `repo` or `admin:repo_hook` (these give unnecessary access)
6. Click **Generate token**
7. **Copy the token immediately** - you won't be able to see it again

### Token Security

- **Keep your token private** - Never commit it to git or share publicly
- **Limited scope** - The token only has `public_repo` scope (can create public issues)
- **Can be revoked** - Delete or regenerate tokens anytime from GitHub Settings
- **Short-lived** - Use an expiration date (90 days recommended)
- **Environment variable** - The token is only stored in memory during the application session

## Without a GitHub Token

If you don't set `CALCHART_GITHUB_TOKEN`, bug reports will:
1. Be formatted as markdown with system information
2. Be automatically copied to your clipboard
3. Allow you to manually paste them into the GitHub issue tracker

This is useful for:
- Testing on development machines
- Users who prefer manual submission
- Offline environments

## Privacy Considerations

When submitting a bug report with or without a GitHub token:

### Always Included
- CalChart version
- Build type (Debug/Release)
- Compiler information
- Build date

### Optional (User-Controlled)
- **System Information** - OS version, architecture, display info, free memory
  - Checkbox to include/exclude (default: included)
- **Show Information** - Sheet count, marcher count, modes, file format
  - Only available if a show is currently open
  - Checkbox to include/exclude (default: included if show open)

### User Can Add
- Email address (optional, for follow-up)
- Custom description and steps to reproduce
- Expected vs actual behavior

### Never Shared
- Password (except what you explicitly type)
- Full file paths beyond general OS info
- Project-specific data not explicitly mentioned in the report
- Personally identifiable information (unless you type it)

## Troubleshooting

### "GitHub API rate limit exceeded"
- GitHub allows 60 API requests/hour unauthenticated
- With a valid token: 5000 requests/hour
- Wait an hour or use a GitHub PAT to increase the limit

### "Authentication failed"
- Verify the token is correct and hasn't expired
- Ensure the token hasn't been revoked in GitHub Settings
- Check that it has the `public_repo` scope

### "Could not access clipboard"
- This is a fallback message on some systems
- You can still manually copy the formatted report

## For CalChart Developers

The GitHub token is read from the environment at submission time:
- See `core/CalChartGitHubIssueSubmitter.cpp` for implementation
- Token is never logged or stored permanently
- Issues are created with labels: `bug`, `user-reported`

## Questions or Issues?

If you have questions about bug reporting, file an issue on GitHub:
[CalChart Issues](https://github.com/calband/calchart/issues)
