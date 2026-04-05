#!/usr/bin/env python3
"""Add visual comments to XBM files showing the bitmap pattern."""

import re
import os
from pathlib import Path


def byte_to_bits(byte_val):
    """Convert a byte to 8 bits, LSB first (XBM format)."""
    bits = []
    for i in range(8):
        bits.append('*' if (byte_val >> i) & 1 else '.')
    return ''.join(bits)


def process_xbm_file(filepath):
    """Process a single XBM file to add visual comments."""
    print(f"Processing {filepath.name}...")
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Extract width and height
    width_match = re.search(r'#define\s+\w+_width\s+(\d+)', content)
    height_match = re.search(r'#define\s+\w+_height\s+(\d+)', content)
    
    if not width_match or not height_match:
        print(f"  Could not find width/height, skipping.")
        return
    
    width = int(width_match.group(1))
    height = int(height_match.group(1))
    bytes_per_row = (width + 7) // 8  # Round up to nearest byte
    
    # Extract the byte values
    array_match = re.search(r'static unsigned char \w+_bits\[\] = \{([^}]+)\}', content, re.DOTALL)
    if not array_match:
        print(f"  Could not find byte array, skipping.")
        return
    
    # Parse hex values
    hex_str = array_match.group(1)
    hex_values = re.findall(r'0x([0-9a-fA-F]+)', hex_str)
    bytes_data = [int(h, 16) for h in hex_values]
    
    if len(bytes_data) != height * bytes_per_row:
        print(f"  Warning: Expected {height * bytes_per_row} bytes but got {len(bytes_data)}")
    
    # Build the new content
    lines = content.split('\n')
    new_lines = []
    
    for line in lines[:2]:  # Keep the #define lines
        new_lines.append(line)
    
    new_lines.append('')
    new_lines.append('// clang-format off')
    
    # Find the array declaration line
    for line in lines:
        if 'static unsigned char' in line and '_bits[]' in line:
            new_lines.append(line)
            break
    
    # Add the bytes with visual comments
    byte_idx = 0
    for row in range(height):
        row_bytes = []
        row_visual = []
        
        for col in range(bytes_per_row):
            if byte_idx < len(bytes_data):
                byte_val = bytes_data[byte_idx]
                row_bytes.append(f'0x{byte_val:02x}')
                row_visual.append(byte_to_bits(byte_val))
                byte_idx += 1
        
        # Combine the visual parts and trim to actual width
        visual = ''.join(row_visual)[:width]
        
        # Format the line (always use trailing comma for consistent alignment)
        bytes_str = ', '.join(row_bytes)
        new_lines.append(f'   {bytes_str},  // {visual}')
    
    new_lines.append('};')
    new_lines.append('// clang-format on')
    new_lines.append('')
    
    # Write the new content
    with open(filepath, 'w') as f:
        f.write('\n'.join(new_lines))
    
    print(f"  ✓ Processed successfully")


def main():
    """Process all XBM files in resources/common directory."""
    script_dir = Path(__file__).parent
    resources_dir = script_dir
    
    if not resources_dir.exists():
        print(f"Error: {resources_dir} does not exist")
        return
    
    xbm_files = sorted(resources_dir.glob('*.xbm'))
    
    if not xbm_files:
        print("No XBM files found")
        return
    
    print(f"Found {len(xbm_files)} XBM files\n")
    
    for xbm_file in xbm_files:
        try:
            process_xbm_file(xbm_file)
        except Exception as e:
            print(f"  Error processing {xbm_file.name}: {e}")
    
    print(f"\nDone!")


if __name__ == '__main__':
    main()
