import glob
import os
import polib
import sys

def copy_msgid_to_msgstr(input_file, copy_all = False):
    # Load the .po file
    po = polib.pofile(input_file)

    # Copy msgid to msgstr
    for entry in po:
        if not entry.msgstr or copy_all:
            entry.msgstr = entry.msgid
        if 'fuzzy' in entry.flags:
            entry.flags.remove('fuzzy')

    # Save to the output file
    po.save()

if __name__ == "__main__":

    if len(sys.argv) < 2:
        print(f"{sys.argv[0]} <language>")
        exit(1)
    
    lang = sys.argv[1]

    copy_all = False
    if lang == 'en':
        copy_all = True
    
    input_file = f"mrv2/po/{lang}.po"
    copy_msgid_to_msgstr(input_file, copy_all)

    plugins_glob=f'mrv2/po/python/plug-ins/locale/{lang}/LC_MESSAGES/*.po'
    for plugin in glob.glob(plugins_glob):
        copy_msgid_to_msgstr(plugin, copy_all)
    
    
