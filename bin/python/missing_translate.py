#!/usr/bin/env python
# encoding: utf-8
#
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script searches .po files for untranslated strings and reports them.
#
# You must run it from the root of the mrv2 project.
#

import sys, os, glob, re


def search_untranslated_strings(pofile):

    in_msg_id = False
    msg = ''
    
    with open(pofile, "r", encoding="utf-8") as fp:
        lines = fp.readlines()

        # Skip header text...
        count = 0
        while count < 14:
            count += 1

        #
        # Look both for fuzzy and untranslated strings
        #
        fuzzy = False
        msgid = ''
        num_lines = len(lines) - 1

        while count < num_lines:
            line = lines[count]
            line2 = lines[count + 1]

            if in_msg_id:
                msgstr = re.match(r'^msgstr\s+"(.*)"$', line)
                match = msgstr.group(1) if msgstr else None

                text2 = re.match(r'^"(.*)"$', line2)
                match2 = text2.group(1) if text2 else None

                if match:
                    if not fuzzy:
                        msgid += match

                if msgstr:
                    if (match == '' or fuzzy) and not match2:
                        print("\tMissing translate for", msgid, "line", count)

                    msgid = ''
                    in_msg_id = fuzzy = False
                    count += 1
                    continue

                count += 1
                continue
            else:
                if "fuzzy" in line:
                    fuzzy = True
                    line = line.replace("fuzzy", '')
                    msgid = ''

            msgid_match = re.match(r'^msgid "(.*)"$', line)
            if msgid_match:
                msgid = msgid_match.group(1)
                in_msg_id = True

            count += 1
            
def search_languages(langs):
    translated = []

    for lang in langs:
        if lang in translated:
            continue

        print("=================== Translate to", lang, "====================")

        #
        # Handle the main mrv2 .po files
        #
        root = "mrv2/po"
        mrv2_po = os.path.join(root, lang + ".po")
        po_files = [mrv2_po]


        #
        # Get list of plugins and their basenames
        #
        plugin_dir = 'mrv2/python/plug-ins'
        plugins = glob.glob(os.path.join(plugin_dir, "*.py"))
        basenames = [os.path.basename(os.path.splitext(f)[0]) for f in plugins]
        
        for plugin in basenames:
            po_file = os.path.join(root, "python", "plug-ins", "locale",
                                   lang, "LC_MESSAGES", plugin + ".po")
            po_files += [po_file]
        
        for po_file in po_files:
            if not os.path.exists(po_file):
                continue

            print(po_file)
            search_untranslated_strings(po_file)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        langs = sys.argv[1:]
    else:
        langs = ['ar', 'cs', 'de', 'en', 'es', 'fr', 'gr', 'it', 'ja',
                 'ko', 'nl', 'pl', 'pt', 'ro', 'ru', 'sv', 'tr', 'zh']

    search_languages(langs)
