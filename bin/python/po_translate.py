#!/usr/env/bin python

from translate import Translator
import polib
import re

import sys
if len(sys.argv) < 2:
    print(sys.argv[0], " <language>")
    exit(1)

lang = sys.argv[1]


# Load the .po file
po = polib.pofile(f'mrv2/po/{lang}.po')

# Initialize your translator
translator = Translator(to_lang=lang)  # Example: translating to French

# Translate each entry
try:
    for entry in po:
        if entry.msgid and not entry.msgstr:
            print("Translate",entry.msgid)
            translated_text = translator.translate(entry.msgid)
            if "WARNING" in translated_text:
                print("Stopping... too many requests for today")
                print(translated_text)
                break
            print("Translated",translated_text)
            entry.msgstr = translated_text
except:
    pass

# Save the translated .po file
po.save(f'mrv2/po/{lang}_translated.po')
