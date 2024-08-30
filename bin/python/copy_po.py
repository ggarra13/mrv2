import glob
import os
import polib
import sys

def copy_msgid_to_msgstr(input_file, output_file):
    # Load the .po file
    po = polib.pofile(input_file)

    # Copy msgid to msgstr
    for entry in po:
        if not entry.msgstr:
            entry.msgstr = entry.msgid
        if 'fuzzy' in entry.flags:
            entry.flags.remove('fuzzy')

    # Save to the output file
    po.save(output_file)

if __name__ == "__main__":

    if len(sys.argv) < 2:
        print(f"{sys.argv[0]} <language>")
        exit(1)
    
    lang = sys.argv[1]
    
    input_file = f"mrv2/po/{lang}.po"
    output_file = f"mrv2/po/{lang}.po"
    copy_msgid_to_msgstr(input_file, output_file)

    plugins_glob=f'mrv2/po/python/plug-ins/locale/{lang}/LC_MESSAGES/*.po'
    for plugin in glob.glob(plugins_glob):
        copy_msgid_to_msgstr(plugin, plugin)
    
    
