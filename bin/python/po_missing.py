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
po_missing v{VERSION}

A program to send all missing .po translations to a file.
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


        
class POMissingTranslations:


    def __init__(self, lang): 
        self.code = lang[0:2]

        if self.code == 'en':
            return
        


    # Load the .po file
    def find_missing_msgstr(self, po_input, po_output):
        if not os.path.exists(po_input):
            print(po_input,'does not exist!')
            exit(1)
            
        po = polib.pofile(po_input)
        out = polib.POFile()
        out.metadata = {
            'Content-Type': 'text/plain; charset=UTF-8',
            'Content-Transfer-Encoding': '8bit',
            'Language': self.code,
            }
        found = False
        try:
            for entry in po:
                if entry.msgstr != '':
                    continue

                found = True
                out.append(entry)
        except Exception as e:
            raise e
        except KeyboardInterrupt:
            pass
        if found:
            print('Saving untraslated strings to',po_output)
            out.save(po_output)
        else:
            print('No untranslated strings for', self.code)


def find_missing_msgstr(lang):

    main_po = f'src/po/{lang}.po'
    out_po = f'src/po/{lang}_missing.po'
    missing = POMissingTranslations(lang)
    missing.find_missing_msgstr(main_po, out_po)
    
    cwd = os.getcwd()
    os.chdir('src/python/plug-ins')
    plugins = glob.glob('*.py')
    os.chdir(cwd)
    for plugin in plugins:
        code   = plugin[:-3]
        plugin = code + '.po'
        out_po = code + '_missing.po'
        plugin_po = f'src/po/python/plug-ins/locale/{lang}/LC_MESSAGES/{plugin}'
        out_po = f'src/po/python/plug-ins/locale/{lang}/LC_MESSAGES/{out_po}'
        missing.find_missing_msgstr(plugin_po, out_po)



if __name__ == "__main__":
    if lang == 'all':
        for lang in LANGUAGES:
            find_missing_msgstr(lang)
    else:
        find_missing_msgstr(lang)    



