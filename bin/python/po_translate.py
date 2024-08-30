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


# Load the model and tokenizer for English to Simplified Chinese
model_name = "Helsinki-NLP/opus-mt-en-zh"
print('Load model',model_name)

tokenizer = MarianTokenizer.from_pretrained(model_name, clean_up_tokenization_spaces=True)
model = MarianMTModel.from_pretrained(model_name)

# Initialize Google translator
translator = Translator(to_lang=lang)


have_seen = {}

# Function to check if translation failed by detecting repeated phrases
def is_translation_invalid(text, threshold=3):
    
    if len(text) > 3 and text[0] == text[2] and text[1] == text[3]:
        if text[0] != text[1]:
            shorten = text[0] + text[1]
        else:
            shorten = text[0]
        return (False, shorten )
    
    # Split the translated text into words/phrases
    words = text.split()
    
    # Count the occurrences of each word/phrase
    word_counts = Counter(words)
    
    # Check if any word/phrase repeats more than the threshold
    for word, count in word_counts.items():
        if count > threshold:
            print('============INVALID')
            return (True, None)
    
    return (False, text)


def translate_text_with_google(english):
    print("Translate with Google...")
    return english
    print(f"Original: {english}")
    translated_text = translator.translate(english)
    if "WARNING" in translated_text:
        print()
        print("Stopping... too many requests for today")
        print()
        return english

def _translate_text(english):
    if english == 'OCIO' or len(english) < 4:
        return english
    
    translated_text = have_seen.get(english, None)

    if not translated_text:
        inputs = tokenizer(english, return_tensors="pt", padding=True,
                           truncation=True)
        
        # Perform the translation
        translated = model.generate(**inputs)

        # Decode the translated tokens
        translated_text = tokenizer.decode(translated[0], skip_special_tokens=True)
    
        # Print the translated text
        result = is_translation_invalid(translated_text)
        if result[0]:
            translated_text = translate_text_with_google(english)
        else:
            translated_text = result[1]
            
        have_seen[english] = translated_text
        
    return translated_text

def translate_text(english):
    if '/' in english and len(english) < 30:
        replaced = english.replace('&','')
        menus = replaced.split('/')
        menus = [_translate_text(menu_item) for menu_item in menus]
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
def translate_po(f):
    po_input = f + '.po'
    po_output = po_input #f + '_translated.po'
    po = polib.pofile(po_input)

    # Translate each entry
    try:
        for entry in po:
            if entry.msgid == entry.msgstr:
                translated = translate_text_with_google(entry.msgid)
                entry.msgstr = translated
            elif entry.msgid and not entry.msgstr:
                translated = translate_text(entry.msgid)
                entry.msgstr = translated

            # Check to see if it begins and ends with newlines
            if entry.msgid[0] == '\n' and entry.msgstr[0] != '\n':
                entry.msgstr = '\n' + entry.msgstr
            if entry.msgid[-1] == '\n' and entry.msgstr[-1] != '\n':
                entry.msgstr = entry.msgstr + '\n'
    except:
        pass

    # Save the translated .po file
    print(f'Saving.... {po_output}')
    po.save(po_output)


#main_po = f'mrv2/po/{lang}'
#translate_po(main_po)
cwd = os.getcwd()
os.chdir('mrv2/python/plug-ins')
plugins = glob.glob('*.py')
os.chdir(cwd)
for plugin in plugins:
    plugin = plugin[:-3]
    plugin_po = f'mrv2/po/python/plug-ins/locale/{lang}/LC_MESSAGES/{plugin}'
    print('Translating plugin',plugin)
    translate_po(plugin_po)
