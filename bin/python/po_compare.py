#
# Built-in imports
#
import argparse
import glob
import os
import polib

#
# Script version
#
VERSION = 1.0

#
# Supported languages
#
LANGUAGES = [
    'de',
    'en',
    'es',
    'fr',
    'hi_IN',
    'it',
    'ja',
    'pt',
    'ru',
    'zh-CN',
]

#
# Script description
#
description=f"""
po_compare v{VERSION}

A program to compare two .po files and spit out the differences in the second one
"""

parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=description)

parser.add_argument('language', type=str,
                    help='Language code to check, like "en" or "zh-CN".')

args = parser.parse_args()
lang = args.language

if not lang in LANGUAGES:
    print(f'Invalid language "{lang}"')
    print(f'Valid ones are:\n\t{", ".join(LANGUAGES)}')
    exit(1)

# Define your variables
original_po = f'src/po/mrv2/{lang}.po'
new_po = f'src/po/mrv2/{lang}_new.po'
diff_po = f'src/po/mrv2/{lang}_diff.txt'

def create_diff_po(original_po_path, new_po_path, diff_po_path):
    if not os.path.exists(original_po_path):
        print(original_po_path, "missing!")
        exit(1)
        
    if not os.path.exists(new_po_path):
        print(new_po_path, "missing!")
        exit(1)

    
    # 1. Load the original and new PO files
    old_file = polib.pofile(original_po_path)
    new_file = polib.pofile(new_po_path)
    
    # 2. Create a new POFile object to store the differences
    diff_file = polib.POFile()
    
    # Optional: copy header metadata from the new file to keep it valid
    diff_file.metadata = new_file.metadata.copy()

    # 3. Build a lookup dictionary for the original PO file
    # We use (msgctxt, msgid) as the key to ensure we match the exact string
    old_entries = {(entry.msgctxt, entry.msgid): entry.msgstr for entry in old_file}

    # 4. Iterate through the new PO file and compare
    diff_count = 0
    for new_entry in new_file:
        key = (new_entry.msgctxt, new_entry.msgid)
        
        # Check if the entry exists in the old file
        if key in old_entries:
            old_msgstr = old_entries[key]
            
            # If the translations don't match, add the new entry to the diff file
            if old_msgstr != new_entry.msgstr:
                print(f'new entry for old {key}')
                diff_file.append(new_entry)
                diff_count += 1
        else:
            # If the entry is entirely new and wasn't in the original file, 
            # we also consider it a difference and add it.
            print(new_entry)
            diff_file.append(new_entry)
            diff_count += 1
            
    # 5. Save the resulting diff to the new file
    diff_file.save(diff_po_path)
    print(f"Comparison complete! Found {diff_count} differing/new entries.")
    print(f"Diff saved to: {diff_po_path}")

# Run the function
if __name__ == "__main__":
    create_diff_po(original_po, new_po, diff_po)
