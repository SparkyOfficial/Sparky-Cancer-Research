#!/usr/bin/env python3
"""
Simple verification script for Protocol Buffer files.
Checks syntax and structure without requiring protoc installation.
"""

import os
import re
import sys

def check_proto_file(filepath):
    """Check a single proto file for basic syntax correctness."""
    errors = []
    warnings = []
    
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
        lines = content.split('\n')
    
    # Check for required syntax declaration
    if not re.search(r'syntax\s*=\s*"proto3"\s*;', content):
        errors.append("Missing or incorrect syntax declaration (should be: syntax = \"proto3\";)")
    
    # Check for package declaration
    if not re.search(r'package\s+\w+\s*;', content):
        warnings.append("Missing package declaration")
    
    # Check for balanced braces
    open_braces = content.count('{')
    close_braces = content.count('}')
    if open_braces != close_braces:
        errors.append(f"Unbalanced braces: {open_braces} open, {close_braces} close")
    
    # Check for balanced parentheses
    open_parens = content.count('(')
    close_parens = content.count(')')
    if open_parens != close_parens:
        errors.append(f"Unbalanced parentheses: {open_parens} open, {close_parens} close")
    
    # Check imports exist
    imports = re.findall(r'import\s+"([^"]+)"\s*;', content)
    for imp in imports:
        import_path = os.path.join('proto', imp)
        if not os.path.exists(import_path):
            errors.append(f"Import not found: {imp}")
    
    # Check for message definitions
    messages = re.findall(r'message\s+(\w+)\s*\{', content)
    if not messages and 'service' not in content:
        warnings.append("No message or service definitions found")
    
    # Check for field numbering
    field_numbers = re.findall(r'=\s*(\d+)\s*;', content)
    if field_numbers:
        numbers = [int(n) for n in field_numbers]
        if len(numbers) != len(set(numbers)):
            warnings.append("Duplicate field numbers detected")
    
    return errors, warnings, messages

def main():
    """Main verification function."""
    proto_dir = 'proto'
    
    if not os.path.exists(proto_dir):
        print(f"Error: {proto_dir} directory not found")
        return 1
    
    proto_files = [
        'common.proto',
        'patient_data.proto',
        'simulation.proto',
        'service.proto'
    ]
    
    print("=" * 60)
    print("Protocol Buffer Files Verification")
    print("=" * 60)
    print()
    
    all_errors = []
    all_warnings = []
    total_messages = 0
    
    for proto_file in proto_files:
        filepath = os.path.join(proto_dir, proto_file)
        
        if not os.path.exists(filepath):
            print(f"❌ {proto_file}: FILE NOT FOUND")
            all_errors.append(f"{proto_file}: File not found")
            continue
        
        print(f"Checking {proto_file}...")
        errors, warnings, messages = check_proto_file(filepath)
        
        if errors:
            print(f"  ❌ {len(errors)} error(s):")
            for error in errors:
                print(f"     - {error}")
            all_errors.extend([f"{proto_file}: {e}" for e in errors])
        else:
            print(f"  ✓ No syntax errors")
        
        if warnings:
            print(f"  ⚠️  {len(warnings)} warning(s):")
            for warning in warnings:
                print(f"     - {warning}")
            all_warnings.extend([f"{proto_file}: {w}" for w in warnings])
        
        if messages:
            print(f"  ✓ {len(messages)} message(s) defined: {', '.join(messages[:5])}")
            if len(messages) > 5:
                print(f"     ... and {len(messages) - 5} more")
            total_messages += len(messages)
        
        print()
    
    print("=" * 60)
    print("Summary")
    print("=" * 60)
    print(f"Files checked: {len(proto_files)}")
    print(f"Total messages: {total_messages}")
    print(f"Errors: {len(all_errors)}")
    print(f"Warnings: {len(all_warnings)}")
    print()
    
    if all_errors:
        print("❌ VERIFICATION FAILED")
        print("\nErrors found:")
        for error in all_errors:
            print(f"  - {error}")
        return 1
    else:
        print("✅ VERIFICATION PASSED")
        print("\nAll proto files have correct basic syntax.")
        print("Note: Full validation requires protoc compiler.")
        if all_warnings:
            print(f"\n{len(all_warnings)} warning(s) - review recommended")
        return 0

if __name__ == '__main__':
    sys.exit(main())
