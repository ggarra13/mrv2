#!/usr/env/bin python

from collections import Counter
import glob
import os
import polib
import re
import sys
from transformers import MarianMTModel, MarianTokenizer
from translate import Translator


if len(sys.argv) < 2:
    print(sys.argv[0], " <language>")
    exit(1)

lang = sys.argv[1]

if lang != 'zh-CN':
    exit(1)


    
DONT_TRANSLATE = [
    'OCIO',
    'ComfyUI',
    'FPS',
    'HUD',
    'sRGB',
]
    
# Load the model and tokenizer for English to Simplified Chinese
model_name = "Helsinki-NLP/opus-mt-en-zh"
print('Load model',model_name)

    
class POTranslator:


    def __init__(self, po_file):
        self.reached_google_limit = False
        self.have_seen = {}
        self.tokenizer = MarianTokenizer.from_pretrained(model_name,
                                                         clean_up_tokenization_spaces=True)
        self.model = MarianMTModel.from_pretrained(model_name)

        # Initialize Google translator
        self.translator = Translator(to_lang=lang)

        # Initialitize po translation
        self.translate_po(po_file)
        
        
    def verify_text(self, input_str):
        # Use regex to match any repeated pair of two characters
        return re.sub(r'(.{2})\1+', r'\1', input_str)


    # Function to check if translation failed by detecting repeated phrases
    def is_translation_invalid(text, threshold=3):
        # Split the translated text into words/phrases
        words = text.split()
    
        # Count the occurrences of each word/phrase
        word_counts = Counter(words)
    
        # Check if any word/phrase repeats more than the threshold
        for word, count in word_counts.items():
            if count > threshold:
                return (True, None)
    
        return (False, text)

    def translate_text_with_google(self, english):
        if self.reached_google_limit:
            return english
        print("Translate with Google...")
        print(f"Original: {english}")
        translated_text = self.translator.translate(english)
        if "WARNING" in translated_text:
            print()
            print("Stopping... too many requests for today")
            print()
            self.reached_google_limit = True
            return english
        print(f'Translated: {translated_text}')
        return translated_text

    
    def _translate_text(self, english):
        if english in DONT_TRANSLATE or len(english) < 4:
            return english
    
        translated_text = self.have_seen.get(english, None)

        if not translated_text:
            inputs = tokenizer(english, return_tensors="pt", padding=True,
                               truncation=True)
        
            # Perform the translation
            translated = model.generate(**inputs)

            # Decode the translated tokens
            translated_text = tokenizer.decode(translated[0],
                                               skip_special_tokens=True)
    
            # Print the translated text
            result = is_translation_invalid(translated_text)
            if result[0]:
                translated_text = translate_text_with_google(english)
            else:
                translated_text = result[1]
            
            self.have_seen[english] = translated_text
        
        return translated_text

    def translate_text(self, english):
        if '/' in english and len(english) < 30:
            replaced = english.replace('&','')
            menus = replaced.split('/')
            menus = [self._translate_text(menu_item) for menu_item in menus]
            translated_text = '/'.join(menus)
            print('MENU English=',replaced)
            print('MENU Translated=', translated_text)
            return translated_text
    
        # Tokenize the input text
        print(f"Original: {english}")
        translated_text = _translate_text(english)
        print(f"Translated: {translated_text}")
        return translated_text

    # Load the .po file
    def translate_po(self, f):
        po_input = f + '.po'
        po_output = po_input #f + '_translated.po'
        po = polib.pofile(po_input)
        
        # Translate each entry
        try:
            for entry in po:
                if entry.msgid == entry.msgstr:
                    translated = self.translate_text_with_google(entry.msgid)
                    entry.msgstr = translated
                elif entry.msgid and not entry.msgstr:
                    translated = self.translate_text(entry.msgid)
                    entry.msgstr = translated

                # Verify repeated two letter characters
                if entry.msgid and entry.msgstr:
                    verify = self.verify_text(entry.msgstr)
                    if verify != entry.msgstr:
                        print("REPEATED:",entry.msgstr)
                        print("     NOW:", verify)
                        entry.msgstr = verify

                # Check to see if it begins and ends with newlines
                if entry.msgid[0] == '\n' and entry.msgstr[0] != '\n':
                    entry.msgstr = '\n' + entry.msgstr
                if entry.msgid[-1] == '\n' and entry.msgstr[-1] != '\n':
                    entry.msgstr = entry.msgstr + '\n'
        except Exception as e:
            raise e

        # Save the translated .po file
        print(f'Saving.... {po_output}')
        po.save(po_output)


main_po = f'mrv2/po/{lang}'
POTranslator(main_po)


cwd = os.getcwd()
os.chdir('mrv2/python/plug-ins')
plugins = glob.glob('*.py')
os.chdir(cwd)
for plugin in plugins:
    plugin = plugin[:-3]
    plugin_po = f'mrv2/po/python/plug-ins/locale/{lang}/LC_MESSAGES/{plugin}'
    print('Translating plugin',plugin)
    POTranslator(plugin_po)
