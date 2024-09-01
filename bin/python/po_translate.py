#!/usr/env/bin python

import argparse
from collections import Counter
import gc
import glob
import os
import polib
import re
import sys

#
# Script version
#
VERSION = 1.0

#
# Supported languages
#
LANGUAGES = [
    'de',
    'hi_IN',
    'zh-CN',
]

#
# Script description
#
description=f"""
po_translate v{VERSION}

A program to translate a .po file with AI and with
Google's translate engine.
"""
            
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=description)

parser.add_argument('language', type=str,
                    help='Language code to translate, like "en" or "zh-CN".')
parser.add_argument('-g', '--google', action='store_true',
                    help='Use google')

args = parser.parse_args()
lang = args.language
use_google = args.google

if not lang in LANGUAGES:
    print(f'Invalid language "{lang}"')
    print(f'Valid ones are:\n\t{", ".join(LANGUAGES)}')
    exit(1)


#
# Text that we should not translate.    
#
DONT_TRANSLATE = [
    '%d Hz.',
    '1:2',
    '1:4',
    '1:8',
    '1:16',
    'API',
    'arib-std-b67',
    'base',
    'bt1361',
    'bt2020',
    'bt2020-10',
    'bt2020-12',
    'bt470m',
    'bt601',
    'bt709',
    'EBU3213',
    'CG',
    'ComfyUI',
    'FFmpeg',
    'FPS',
    'Gamma',
    'gamma',
    'gamma28',
    'gamma22',
    'gtk+',
    'HDR',
    'HUD',
    'iec61966-2-1',
    'iec61966-2-4',
    'Laser',
    'Linear',
    'Lumma',
    'OCIO',
    'OpenEXR',
    'OpenGL',
    'OpenUSD',
    'oxy',
    'plastic',
    'SMPTE170m',
    'smpte2084',
    'SMPTE240m',
    'smpte240m',
    'SMPTE428',
    'SMPTE431',
    'SMPTE432',
    'sRGB',
    'Stereo',
    'Stereo3D',
    'Stereo 3D',
    'Studio',
    'USD',
]

code = lang[0:2]


#
# Google Languages from their two letter code
#
GOOGLE_LANGUAGES = {
    'en' : 'English',
    'es' : 'Spanish',
    'it' : 'Italian',
    'de' : 'German',
    'hi' : 'Hindi',
    'zh' : 'Chinese (Simplified)',
}

#
# Load the heavy imports
#
import torch
from transformers import MarianMTModel, MarianTokenizer
from translate import Translator

# Load the model and tokenizer for English to Simplified Chinese
model_name = f"Helsinki-NLP/opus-mt-en-{code}"
print('Load model',model_name)

    
class POTranslator:


    def __init__(self, po_file, use_google):
        self.reached_google_limit = not use_google
        self.have_seen = {}
        self.tokenizer = MarianTokenizer.from_pretrained(model_name,
                                                         clean_up_tokenization_spaces=True)
        self.model = MarianMTModel.from_pretrained(model_name)

        # Initialize Google translator
        self.translator = Translator(to_lang=GOOGLE_LANGUAGES[code])

        # Initialitize po translation
        self.translate_po(po_file)
        
    def verify_text(self, text):
        # Use regex to match any repeated pair of two characters
        if lang == 'zh-CN':
            text = re.sub(r'(.{2})\1+', r'\1', text)

        # Split the translated text into words
        words = text.split()

        # Avoid repeated words
        norepeats = []
        prev_word = ''
        for i in range(len(words)):
            if words[i] == prev_word:
                continue
            norepeats.append(words[i])
            prev_word = words[i]
        
        text = ' '.join(norepeats)
        return text

    def is_number(self, s):
        return re.fullmatch(r'^-?\d+(\.\d+)?$', s)

    # Function to check if translation failed by detecting repeated phrases
    def is_translation_invalid(self, text, threshold=3):
        # Split the translated text into words/phrases
        words = text.split()
    
        # Count the occurrences of each word/phrase
        word_counts = Counter(words)
    
        # Check if any word/phrase repeats more than the threshold
        for word, count in word_counts.items():
            if count > threshold:
                return (True, None)
    
        return (False, text)

    def translate_with_google(self, english):
        if len(english) < 4:
            return english

        if self.is_number(english):
            return english
    
        if self.reached_google_limit:
            return english

        print('GOOGLE TRANSLATE:')
        print(f"\tOriginal: {english}")
        translated_text = self.translator.translate(english)
        print(f'\tTranslated: {translated_text}')
        if "WARNING" in translated_text or 'http' in translated_text:
            print()
            print("Stopping... too many requests for today")
            print()
            self.reached_google_limit = True
            return english
        return translated_text

    
    def _translate_text(self, english):
        if english in DONT_TRANSLATE:
            return english
        
        if len(english) < 4:
            return english

        if self.is_number(english):
            return english
    
        translated_text = self.have_seen.get(english, None)

        if not translated_text:
            inputs = self.tokenizer(english, return_tensors="pt", padding=True,
                                    truncation=True)
        
            # Perform the translation
            translated = self.model.generate(**inputs)

            # Decode the translated tokens
            translated_text = self.tokenizer.decode(translated[0],
                                                    skip_special_tokens=True)
    
            # Print the translated text
            result = self.is_translation_invalid(translated_text)
            if result[0]:
                translated_text = self.translate_with_google(english)
            else:
                translated_text = result[1]

            verify = self.verify_text(translated_text)
            if verify != translated_text:
                print("\tREPEATED:", translated_text)
                print("\t     NOW:", verify)
                translated_text = verify
            
            self.have_seen[english] = translated_text
        
        return translated_text

    #
    # Main translate text function.  Checks if it is a menu and translates
    # each menu entry separately.
    #
    def translate_text(self, english):
        print('AI TRANSLATE')
        if '/' in english and len(english) < 40:
            replaced = english.replace('&','')
            menus = replaced.split('/')
            menus = [self._translate_text(menu_item) for menu_item in menus]
            translated_text = '/'.join(menus)
            print('\t\tMENU English=',replaced)
            print('\t\tMENU Translated=', translated_text)
            return translated_text
    
        # Tokenize the input text
        print(f"\tOriginal: {english}")
        translated_text = self._translate_text(english)
        if 'QUERY LENGTH' in translated_text:
            return english
        print(f"\tTranslated: {translated_text}")
        return translated_text

    # Load the .po file
    def translate_po(self, f):
        po_input = f + '.po'
        po_output = po_input #f + '_translated.po'
        po = polib.pofile(po_input)
        
        # Translate each entry
        try:
            for entry in po:
                if entry.msgid in DONT_TRANSLATE:
                    entry.msgstr = entry.msgid
                elif 'GOOGLE' == entry.msgstr:
                    translated = self.translate_with_google(entry.msgid)
                    entry.msgstr = translated
                elif entry.msgid and not entry.msgstr:
                    translated = self.translate_text(entry.msgid)
                    entry.msgstr = translated
                if 'fuzzy' in entry.flags:
                    translated = self.translate_text(entry.msgid)
                    entry.msgstr = translated
                    entry.flags.remove('fuzzy')

                # Check to see if it begins and ends with newlines
                if entry.msgid[0] == '\n' and entry.msgstr[0] != '\n':
                    entry.msgstr = '\n' + entry.msgstr
                if entry.msgid[-1] == '\n' and entry.msgstr[-1] != '\n':
                    entry.msgstr = entry.msgstr + '\n'
        except Exception as e:
            pass
        except KeyboardInterrupt:
            pass

        # Save the translated .po file
        print(f'Saving.... {po_output}')
        po.save()

    def __del__(self):
        # Unload the model to free memory
        del self.model
        del self.tokenizer
        
main_po = f'mrv2/po/{lang}'
POTranslator(main_po, use_google)


cwd = os.getcwd()
os.chdir('mrv2/python/plug-ins')
plugins = glob.glob('*.py')
os.chdir(cwd)
for plugin in plugins:
    plugin = plugin[:-3]
    plugin_po = f'mrv2/po/python/plug-ins/locale/{lang}/LC_MESSAGES/{plugin}'
    print('Translating plugin',plugin)
    POTranslator(plugin_po, use_google)


# Clear cached data in PyTorch
torch.cuda.empty_cache()  # Clears GPU memory if used

# Run garbage collection
gc.collect()

