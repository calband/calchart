#!/bin/bash
# Setup script for calchart-viewer submodule
#
# Note: CMake will build the viewer automatically if Node.js is installed.
# This script is provided for manual builds or troubleshooting.

set -e

echo "Setting up CalChart Viewer..."
echo ""
echo "Note: CMake builds the viewer automatically during the build process."
echo "This script is only needed for manual builds or troubleshooting."
echo ""

# Check if we're in the right directory
if [ ! -f "viewer/package.json" ]; then
    echo "Error: Run this script from the CalChart root directory"
    exit 1
fi

# Check for Node.js
if ! command -v node &> /dev/null; then
    echo "Error: Node.js is not installed"
    echo "Please install Node.js from https://nodejs.org/"
    exit 1
fi

# Check for npm
if ! command -v npm &> /dev/null; then
    echo "Error: npm is not installed"
    echo "npm should come with Node.js. Please reinstall Node.js."
    exit 1
fi

cd viewer

echo "Installing npm dependencies..."
npm install

echo "Installing grunt-cli globally (may require sudo)..."
npm install -g grunt-cli 2>/dev/null || sudo npm install -g grunt-cli

echo "Building viewer..."
grunt build

echo ""
echo "✅ Viewer setup complete!"
echo ""
echo "To auto-rebuild during development, run:"
echo "  cd viewer && grunt watch"
echo ""
