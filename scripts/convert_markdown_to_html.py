#!/usr/bin/env python3
"""
Convert Markdown files to HTML for CalChart help system.

This script recursively converts all .md files in a directory to .html files,
preserving the directory structure using pandoc.

Requires pandoc to be installed:
  - macOS: brew install pandoc
  - Windows: choco install pandoc
  - Linux: sudo apt-get install pandoc

Usage:
    python3 convert_markdown_to_html.py <input_dir> <output_dir>
"""

import sys
import subprocess
import shutil
import re
from pathlib import Path


def has_pandoc():
    """Check if pandoc is installed."""
    return shutil.which('pandoc') is not None


def convert_with_pandoc(md_file, html_file, input_dir):
    """Convert Markdown to HTML using pandoc."""
    try:
        subprocess.run([
            'pandoc',
            md_file,
            '-f', 'markdown',
            '-t', 'html5',
            '-o', html_file,
            '--standalone',
            '--embed-resources',
            '--metadata', 'title=CalChart Help',
        ], check=True, capture_output=True)
        
        # Post-process: Convert .md links to help:// protocol links
        with open(html_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Replace href="something.md" with href="help://something"
        # Handle relative paths by resolving them relative to the current file
        def replace_md_link(match):
            link = match.group(1)
            
            # Resolve relative path from the current HTML file's directory
            html_dir = Path(html_file).parent
            # Resolve the link relative to the HTML file's directory
            resolved = (html_dir / link).resolve()
            
            # Get the path relative to the output directory root
            try:
                output_dir = Path(html_file).parent
                while output_dir.name != 'html' and output_dir.parent != output_dir:
                    output_dir = output_dir.parent
                
                if output_dir.name == 'html':
                    rel_to_root = resolved.relative_to(output_dir)
                else:
                    # Fallback: just use the link as-is with .. resolved
                    rel_to_root = Path(link)
                    
                # Remove .md extension and convert to forward slashes
                topic_id = str(rel_to_root).replace('.md', '').replace('\\', '/')
            except (ValueError, Exception):
                # If we can't resolve it, fall back to simple replacement
                topic_id = link.replace('.md', '')
            
            return f'href="help://{topic_id}"'
        
        content = re.sub(r'href="([^"]+\.md)"', replace_md_link, content)
        
        with open(html_file, 'w', encoding='utf-8') as f:
            f.write(content)
        
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error converting {md_file}: {e.stderr.decode()}")
        return False
    except Exception as e:
        print(f"Error post-processing {html_file}: {e}")
        return False





def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input_dir> <output_dir>")
        sys.exit(1)
    
    input_dir = Path(sys.argv[1])
    output_dir = Path(sys.argv[2])
    
    if not input_dir.is_dir():
        print(f"Error: Input directory not found: {input_dir}")
        sys.exit(1)
    
    # Create output directory
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Check for pandoc (required)
    if not has_pandoc():
        print("Error: pandoc is required but not found.")
        print("Please install pandoc:")
        print("  - macOS: brew install pandoc")
        print("  - Windows: choco install pandoc")
        print("  - Linux: sudo apt-get install pandoc")
        sys.exit(1)
    
    print("Using pandoc for conversion")
    
    # Find all .md files
    md_files = list(input_dir.rglob('*.md'))
    
    if not md_files:
        print(f"No Markdown files found in {input_dir}")
        return
    
    converted = 0
    failed = 0
    
    for md_file in md_files:
        # Skip files in _resources directories
        if '_resources' in md_file.parts:
            continue
        
        # Calculate relative path and create output structure
        rel_path = md_file.relative_to(input_dir)
        html_file = output_dir / rel_path.with_suffix('.html')
        
        # Create parent directories
        html_file.parent.mkdir(parents=True, exist_ok=True)
        
        print(f"Converting {rel_path}...", end=" ")
        
        if convert_with_pandoc(str(md_file), str(html_file), input_dir):
            print("OK")
            converted += 1
        else:
            print("FAILED")
            failed += 1
    
    print(f"\nConversion complete: {converted} succeeded, {failed} failed")
    
    if failed > 0:
        sys.exit(1)


if __name__ == '__main__':
    main()

