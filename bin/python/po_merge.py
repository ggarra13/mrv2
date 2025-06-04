#!/usr/env/bin python

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
po_merge v{VERSION}

A program to merge a missing .po file into the main one.
"""
            
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=description)

parser.add_argument('language', type=str,
                    help='Language code to check, like "en" or "zh-CN".')

args = parser.parse_args()
lang = args.language

if not lang in ['all'] + LANGUAGES:
    print(f'Invalid language "{lang}"')
    print(f'Valid ones are:\n\t{", ".join(LANGUAGES)}')
    exit(1)


        
class POMergeMissing:
    def __init__(self): 
        pass

    # Load the .po file
    def merge(self, merged_file, main_po, missing_po):
        if not os.path.exists(main_po):
            print(main_po,'does not exist!')
            exit(1)
            
        if not os.path.exists(missing_po):
            print(missing_po,'does not exist!')
            return
            
        # Load both PO files
        source_po = polib.pofile(missing_po)  # The PO file containing the replacement translations
        target_po = polib.pofile(main_po)  # The PO file you want to modify

        # Create a dictionary of msgid -> msgstr from the source PO file
        # for quick lookup
        source_entries = {entry.msgid: entry.msgstr for entry in source_po}

        # Iterate over the entries in the target PO file
        for entry in target_po:
            if entry.msgid in source_entries:
                # Replace the msgstr in the target PO file with the one from the source PO file
                entry.msgstr = source_entries[entry.msgid]
                entry.fuzzy = False

        # Save the modified target PO file
        print('Creating merged',merged_file)
        target_po.save(merged_file)


def merge_missing(lang):

    main_po = f'src/po/mrv2/{lang}.po'
    merged_po = f'src/po/mrv2/{lang}.po'
    missing_po = f'src/po/mrv2/{lang}_missing.po'
    missing = POMergeMissing()
    missing.merge(merged_po, main_po, missing_po)
    
    cwd = os.getcwd()
    os.chdir('src/python/plug-ins')
    plugins = glob.glob('*.py')
    os.chdir(cwd)
    for plugin in plugins:
        code   = plugin[:-3]
        merged = code + '.po'
        plugin = code + '.po'
        out_po = code + '_missing.po'
        merged_po = f'src/po/mrv2/python/plug-ins/locale/{lang}/LC_MESSAGES/{merged}'
        plugin_po = f'src/po/mrv2/python/plug-ins/locale/{lang}/LC_MESSAGES/{plugin}'
        out_po = f'src/po/mrv2/python/plug-ins/locale/{lang}/LC_MESSAGES/{out_po}'
        missing.merge(merged_po, plugin_po, out_po)

if __name__ == "__main__":
    if lang == 'all':
        for lang in LANGUAGES:
            merge_missing(lang)
    else:
        merge_missing(lang)    



