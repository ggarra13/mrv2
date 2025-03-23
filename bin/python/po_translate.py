#!/usr/env/bin python

#
# Built-in imports
#
import argparse
from collections import Counter
import gc
import glob
import os
import polib
import re
import sys
from urllib.error import HTTPError

os.environ["TOKENIZERS_PARALLELISM"] = "false"

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
po_translate v{VERSION}

A program to translate a .po file with AI and with
Google's translate engine.
"""
            
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description=description)

parser.add_argument('language', type=str,
                    help='Language code to translate, like "en" or "zh-CN".')
parser.add_argument('-g', '--use-google', action='store_true',
                    help='Use google')
parser.add_argument('-nt', '--no-tokenizer', action='store_true',
                    help='Do not use AI tokenizer')

args = parser.parse_args()
lang = args.language
use_google = args.use_google
use_tokenizer = not args.no_tokenizer

if not lang in ['all'] + LANGUAGES:
    print(f'Invalid language "{lang}"')
    print(f'Valid ones are:\n\t{", ".join(LANGUAGES)}')
    exit(1)


#
# Text that we should not translate.    
#
DONT_TRANSLATE = [
    '@B12@C7@b@.Remote\\t@B12@C7@b@.Local'
    '70 mm',
    '35 mm Panavision',
    'HDR maxCLL',
    'HDR maxFALL',
    'FourCC: {0}',
    '%d Hz.',
    '1:2',
    '1:4',
    '1:8',
    '1:16',
    'x1',
    'x2',
    'x3',
    'x4',
    'x5',
    'x6',
    'x7',
    'x8',
    'x9',
    '24',
    '1/2',
    '1/3',
    '1/4',
    '1/5',
    '1/6',
    '1/7',
    '1/8',
    '1/9',
    '24',
    'Alt',
    'API',
    'arib-std-b67',
    'base',
    'BGR',
    'RGB',
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
    'linear',
    'log',
    'logsqrt',
    'Lumma',
    'Meta',
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
    'ZIP',
    'ZIPS',
]

#
# Text that we should translate differently due to mix-ups with several
# meanings in English language.    
#
TRANSLATE_FIXES = {
    'float' : 'floating point',
    'Float' : 'Floating point',
}


#
# Google Languages from their two letter code
#
GOOGLE_LANGUAGES = {
    'en' : 'English',
    'es' : 'Spanish',
    'it' : 'Italian',
    'de' : 'German',
    'fr' : 'French',
    'hi' : 'Hindi',
    'it' : 'Italian',
    'ja' : 'Japanese',
    'pt' : 'Portuguese',
    'ru' : 'Russian',
    'zh' : 'ZH-CN',
}

#
# Load the heavy imports
#
try:
    if use_tokenizer:
        import torch
        from transformers import MarianMTModel, MarianTokenizer
except ImportError:
    use_tokenizer = False
    pass


if not use_google and not use_tokenizer:
    use_google = True
    print('Use google and use tokenizer are false.  Turning on google')

try:
    from translate import Translator
except ImportError:
    use_google = False
    pass

if not use_google and not use_tokenizer:
    print('No google or tokenizer module found.  Exiting.')
    exit(1)

        
class POTranslator:


    def __init__(self, lang, use_google = False, use_tokenizer = True): 
        self.model = self.tokenizer = None
        
        self.use_google = use_google
        self.use_tokenizer = use_tokenizer
        self.code = lang[0:2]
        # Load the model and tokenizer for English to Simplified Chinese
        self.helsinki = self.code
        if self.code == 'pt' or self.code == 'fr' or \
           self.code == 'es' or self.code == 'it':
            self.helsinki = 'ROMANCE'
        elif self.code == 'ja':
            self.helsinki = 'jap'
        
        self.have_seen = {}

        if self.code == 'en':
            return
    
        
        if use_tokenizer:
            model_name = f"Helsinki-NLP/opus-mt-en-{self.helsinki}"
            print('Load model',model_name,'for',self.code)
            self.tokenizer = MarianTokenizer.from_pretrained(model_name,
                                                             clean_up_tokenization_spaces=True)
            try:
                self.model = MarianMTModel.from_pretrained(model_name)
            except HTTPError as e:
                print(f'Model {model_name} not found!')
                exit(1)

        # Initialize Google translator
        if use_google:
            self.translator = Translator(to_lang=GOOGLE_LANGUAGES[self.code])
        
        
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
        if self.have_seen.get(english, None):
            return self.have_seen[english]
        
        if len(english) < 4 and not self.is_number(english):
            return english
    
        if not self.use_google:
            return ''

        print('GOOGLE TRANSLATE:')
        print(f"\tOriginal: {english}")
        try:
            translated_text = self.translator.translate(english)
            print(f'\tTranslated: {translated_text}')
            self.have_seen[english] = translated_text
        except Exception as e:
            print(e)
            translated_text = ''
        if "WARNING" in translated_text:
            print()
            print("Stopping... too many requests for today")
            print(translated_text)
            self.use_google = False
            
            if 'http' in translated_text:
                translated_text = re.sub('<a href.*</a>', '', translated_text)

        if english == translated_text:
            print()
            print('GOOGLE TRANSLATE FAILED.')
            print()
            return ''
        return translated_text

    
    def _translate_text(self, english):
        if english in DONT_TRANSLATE:
            return english
        
        if len(english) < 4 and not self.is_number(english):
            print(english)
            return english
    
        translated_text = self.have_seen.get(english, None)
        
        if not translated_text:
            if not self.use_tokenizer:
                return self.translate_with_google(english)

            text = english
            if self.helsinki == 'ROMANCE':
                text = f'>>{self.code}<< ' + text
            
            inputs = self.tokenizer(text, return_tensors="pt", padding=True,
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
                translated_text = verify
            
            self.have_seen[english] = translated_text
        
        return translated_text

    #
    # Main translate text function.  Checks if it is a menu and translates
    # each menu entry separately.
    # If we have '/' try to translate with google first 
    #
    def translate_text(self, english):
        if '/' in english:
            return ''
        if ':class:' in english:
            return ''
        if '(*.' in english:
            return ''
        if self.code == 'ja' and self.use_google:
            return self.translate_with_google(english)        
        if '\n' in english:
            if self.use_google:
                return self.translate_with_google(english)
            lines = english.split('\n')
            lines = [self._translate_text(line) for line in lines]
            translated_text = '\n'.join(lines)
            print('\t\tNEWLINES English=',english)
            print('\t\tNEWLINES Translated=', translated_text)
            return translated_text
        
        if '/' in english and len(english) < 40:
            if self.use_google:
                return self.translate_with_google(english)
            replaced = english.replace('&','')
            menus = replaced.split('/')
            menus = [self._translate_text(menu_item) for menu_item in menus]
            translated_text = '/'.join(menus)
            print('\t\tMENU English=',replaced)
            print('\t\tMENU Translated=', translated_text)
            return translated_text

        if not self.tokenizer and not self.use_google:
            return ''
        
        # Tokenize the input text
        print(f"\tTRANSFORMERS Original: {english}")

        if TRANSLATE_FIXES.get(english, False):
            english = TRANSLATE_FIXES[english]
        
        translated_text = self._translate_text(english)
        if 'QUERY LENGTH' in translated_text:
            return '********FAILED********'
        translated_text = translated_text.replace('Mrv2', 'mrv2')
        print(f"\tTranslated: {translated_text}")
        if translated_text == english:
            return ''
        return translated_text

    # Load the .po file
    def translate_po(self, po_input):
        if not os.path.exists(po_input):
            print(po_input,'does not exist!')
            exit(1)
            
        po = polib.pofile(po_input)
        
        # Translate each entry
        language = self.code
        try:
            for entry in po:
                if language == 'en':
                    entry.msgstr = entry.msgid
                    if 'fuzzy' in entry.flags:
                        entry.flags.remove('fuzzy')
                    continue
                elif entry.msgid in DONT_TRANSLATE:
                    print('DONT TRANSLATE')
                    entry.msgstr = entry.msgid
                elif entry.msgid and not entry.msgstr:
                    translated = self.translate_text(entry.msgid)
                    entry.msgstr = translated
                elif entry.msgid == entry.msgstr:
                    translated = self.translate_text(entry.msgid)
                    entry.msgstr = translated
                if 'fuzzy' in entry.flags:
                    entry.flags.remove('fuzzy')

                if entry.msgstr == '':
                    continue
                    
                # Check to see if it begins and ends with newlines
                if entry.msgid[0] == '\n' and entry.msgstr[0] != '\n':
                    entry.msgstr = '\n' + entry.msgstr
                if entry.msgid[-1] == '\n' and entry.msgstr[-1] != '\n':
                    entry.msgstr = entry.msgstr + '\n'
        except Exception as e:
            raise e
        except KeyboardInterrupt:
            pass

        # Save the translated .po file
        print(f'Saving.... {po_input}')
        po.save()

    def __del__(self):
        # Unload the model to free memory
        if self.model:
            del self.model
        if self.tokenizer:
            del self.tokenizer

def po_translate(lang):

    main_po = f'src/po/{lang}.po'
    translator = POTranslator(lang, use_google, use_tokenizer)
    translator.translate_po(main_po)

    cwd = os.getcwd()
    os.chdir('src/python/plug-ins')
    plugins = glob.glob('*.py')
    os.chdir(cwd)
    for plugin in plugins:
        plugin = plugin[:-3] + '.po'
        plugin_po = f'src/po/python/plug-ins/locale/{lang}/LC_MESSAGES/{plugin}'
        print('Translating plugin',plugin)
        translator.translate_po(plugin_po)


model_name = ''

if __name__ == "__main__":
    import multiprocessing
    multiprocessing.freeze_support()

    if lang == 'all':
        for lang in LANGUAGES:
            po_translate(lang)
    else:
        po_translate(lang)    




# Clear cached data in PyTorch
if use_tokenizer:
    torch.cuda.empty_cache()  # Clears GPU memory if used

# Run garbage collection
gc.collect()

