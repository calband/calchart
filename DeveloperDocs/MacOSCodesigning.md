# Codesigning and Notorizing for Mac:

These are the steps you need to do before we run CI to Codesign and Notor
ize.

**Prerequisites:** You need an Apple Developer Account (Apple ID)

## Step 1: Generate a Certificate Signing Request (CSR)

On your Mac, open Keychain Access.

In the menu bar, select:
```
    Keychain Access → Certificate Assistant → Request a Certificate From 
a Certificate Authority…
```

Fill in:
 * **User Email Address:** your Apple ID
 * **Common Name:** something like Developer ID for CalChart
 * **CA Email:** leave blank
 * **Request is:** ✔ Save to disk

Click Continue, and save the `.certSigningRequest` file.

## Step 2: Create a Developer ID Application Certificate

 * Go to [https://developer.apple.com/account/](https://developer.apple.com/account/)
 * Navigate to Certificates, Identifiers & Profiles
 * Under Certificates, click the ➕ button
 * Choose: `Type: Developer ID Application`
 * Click Continue
 * Upload your `.certSigningRequest`
 * Click Continue, then Download the `.cer` file

## Step 3: Import the Certificate to Keychain

Double-click the downloaded `.cer` file.  It will appear in your login keychain. Make sure the certificate appears with a private key (expand the triangle in Keychain).

If there's no private key: Something went wrong during CSR generation. Try again from Step 1.

## Step 4: Export as .p12 for GitHub Use

 * In Keychain Access, right-click the certificate → Export. Choose `.p12` format.
 * Set a strong password (you’ll store this in GitHub Secrets as `MACOS_CERTIFICATE_PASSWORD`).
 * Save the file as `DeveloperID.p12`

## Step 5: App-Specific Password for notarytool

`notarytool` is going to access the server as you, so create an app-specific password.  If you haven’t already, create an App-Specific Password for your Apple ID.

[https://account.apple.com/account/manage](https://account.apple.com/account/manage).
 
Create a password for notarytool (you’ll store this in GitHub Secrets as APP_SPECIFIC_PASSWORD)

## Step 6: Base64 Encode and Store in GitHub
Run:
```
base64 DeveloperID.p12 | pbcopy
```

Then in GitHub, Go to Settings → Secrets and variables → Actions. Add these secrets:
 * MACOS_CERTIFICATE (Paste base64 contents)
 * MACOS_CERTIFICATE_PASSWORD (The password you set for export)
 * DEVELOPER_ID_APP "Developer ID Application: Your Name (TEAMID)"
 * APPLE_ID (Your developer Apple ID)
 * APPLE_TEAM_ID (Your developer team (usually 10 alphanumeric digits))
 * APP_SPECIFIC_PASSWORD (The notarytool password)

To get the full identity name, run:
```
    security find-identity -p codesigning -v
```
  You'll see:

```
    1) XXXXXXXX "Developer ID Application: Richard Powell (ABCDE12345)"
```

Use that full quoted string in `DEVELOPER_ID_APP`.

