# Windows Code Signing Guide

## Overview

Windows code signing eliminates the "Windows protected your PC" SmartScreen warning by cryptographically signing your executable with a trusted certificate. This proves the software comes from a verified publisher.

## Why Sign Your Application?

- **Eliminates SmartScreen warnings**: Users won't see scary warnings when downloading/running CalChart
- **Builds trust**: Shows you are a verified publisher
- **Prevents tampering**: Digital signature detects any modifications to the executable
- **Required for some enterprise environments**: Many organizations block unsigned software

## Prerequisites

### 1. Obtain a Code Signing Certificate

You have two options:

#### Option A: EV (Extended Validation) Certificate (Recommended)
- **Cost**: $300-500/year
- **Advantage**: Immediate SmartScreen reputation - no warnings from day one
- **Providers**:
  - [DigiCert](https://www.digicert.com/signing/code-signing-certificates)
  - [SSL.com](https://www.ssl.com/certificates/ev-code-signing/)
  - [Sectigo](https://sectigo.com/ssl-certificates-tls/code-signing)

#### Option B: Standard Code Signing Certificate
- **Cost**: $100-300/year
- **Advantage**: Lower cost
- **Disadvantage**: Must build reputation over time (downloads/installs) before SmartScreen warnings disappear
- **Same providers as above**

### 2. Identity Verification

Both certificate types require identity verification:
- **For individuals**: Government-issued ID, address verification
- **For companies**: Business registration documents, DUNS number (for EV)
- **Processing time**: 1-7 business days (EV takes longer)

### 3. Certificate Format

- Request the certificate in **PFX (PKCS#12)** format with the private key
- Set a strong password when exporting the certificate
- Store the certificate securely - it proves your identity!

## Setting Up Code Signing for GitHub Actions

### Step 1: Export and Encode Certificate

**On Windows (PowerShell):**
```powershell
# Convert PFX to base64
$fileContent = Get-Content -Path "certificate.pfx" -Encoding Byte
$base64 = [System.Convert]::ToBase64String($fileContent)
$base64 | Set-Clipboard
```

**On Linux/Mac (Git Bash):**
```bash
base64 -w 0 certificate.pfx | clip  # Windows
base64 certificate.pfx | pbcopy     # macOS
```

### Step 2: Add GitHub Secrets

1. Go to your repository on GitHub
2. Navigate to **Settings** → **Secrets and variables** → **Actions**
3. Click **New repository secret**
4. Add the following secrets:

| Secret Name | Description |
|-------------|-------------|
| `WINDOWS_CERTIFICATE` | The base64-encoded PFX certificate (paste from clipboard) |
| `WINDOWS_CERTIFICATE_PASSWORD` | The password for the PFX file |

### Step 3: Verify CI Configuration

The `.github/workflows/cmake.yml` file has been updated with:
- Certificate import step
- Executable signing step (CalChart.exe)
- Installer signing step (CalChart-*.exe installer package)

The signing only runs on:
- Windows builds
- Non-pull-request events (pushes to main/tags)
- When the `WINDOWS_CERTIFICATE` secret exists

## Local Development / Manual Signing

### Sign an Executable Locally

```powershell
# Find signtool (part of Windows SDK)
$signtool = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe"

# Sign with timestamp
& $signtool sign `
  /f "certificate.pfx" `
  /p "YourPassword" `
  /tr http://timestamp.digicert.com `
  /td SHA256 `
  /fd SHA256 `
  CalChart.exe

# Verify signature
& $signtool verify /pa /v CalChart.exe
```

### Verify a Signature

**PowerShell:**
```powershell
Get-AuthenticodeSignature CalChart.exe | Format-List *
```

**Windows Explorer:**
- Right-click the .exe → Properties → Digital Signatures tab

## Timestamp Servers

Always use a timestamp server when signing. This allows signatures to remain valid after the certificate expires.

Common timestamp servers:
- DigiCert: `http://timestamp.digicert.com`
- Sectigo: `http://timestamp.sectigo.com`
- GlobalSign: `http://timestamp.globalsign.com`

## Certificate Management

### Security Best Practices

1. **Never commit certificates to version control**
2. **Use strong passwords** for PFX files
3. **Rotate certificates** before expiration (most are 1-3 years)
4. **Revoke immediately** if the certificate is compromised
5. **Limit access** to the certificate - only trusted CI/release managers

### Certificate Renewal

Certificates typically expire after 1-3 years. Before expiration:

1. Purchase a renewed certificate from your CA
2. Update the GitHub secrets with the new certificate
3. Test the signing process
4. The old signature remains valid on previously-released versions

## Troubleshooting

### "SignTool Error: No certificates were found that met all the given criteria"
- Certificate not properly imported
- Password incorrect
- Certificate expired

### "The specified timestamp server could not be reached"
- Temporary network issue - retry
- Try a different timestamp server
- Check firewall/proxy settings

### "After signing, SmartScreen still shows warnings"
- **EV certificates**: Should work immediately
- **Standard certificates**: Need to build reputation (100s-1000s of downloads/installs over weeks)
- Verify signature is valid: `signtool verify /pa /v CalChart.exe`

### "Failed to sign executable. Exit code: 1"
- Check certificate password is correct in GitHub Secrets
- Ensure certificate hasn't expired
- Verify PFX file is properly base64 encoded

## Cost-Benefit Analysis

| Certificate Type | Annual Cost | Time to Trust | Best For |
|------------------|-------------|---------------|----------|
| **EV Code Signing** | $300-500 | Immediate | Production releases, commercial software |
| **Standard Code Signing** | $100-300 | 2-8 weeks | Open source projects, lower volume software |
| **No Signing** | $0 | Never | Development only, internal use |

## Additional Resources

- [Microsoft: Introduction to Code Signing](https://docs.microsoft.com/en-us/windows/win32/seccrypto/cryptography-tools)
- [DigiCert: Code Signing Best Practices](https://docs.digicert.com/en/software-trust-manager/ci-cd-integrations/script-integrations/github-integration.html)
- [Windows Dev Center: App Signing](https://docs.microsoft.com/en-us/windows/msix/package/sign-app-package-using-signtool)

## Summary

Code signing is essential for professional Windows software distribution. While it requires an annual investment, it dramatically improves user trust and eliminates security warnings that can prevent users from installing your application.

For CalChart's use case (open source, educational), a **standard code signing certificate (~$150/year)** is a reasonable middle ground that will build trust over time.
