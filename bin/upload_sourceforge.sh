#!/usr/bin/env bash

. etc/functions.sh

# Create the remote directory
ssh -i $HOME/.ssh/id256_rsa ggarra13@frs.sourceforge.net 'mkdir -p /home/frs/project/mrv2/beta/'


upload_file()
{
    # Upload the file to the created directory
    echo
    echo "Sending $1 as $2..."
    echo
    rsync -avP --ignore-errors -e "ssh -i $HOME/.ssh/id256_rsa" $1 ggarra13@frs.sourceforge.net:/home/frs/project/mrv2/beta/$2
}

# Extract cmake/version.cmake into mrv2_VERSION
extract_version

echo "mrv2 VERSION=$mrv2_VERSION"

cd packages

files=`ls -1 *v${mrv2_VERSION}*`


date=$(date "+%Y-%B-%d %H:%M:%S")
cat <<EOF > upload_date.txt

mrv2 Beta
---------

This beta release corresponds to ${date}.

It may contain bugs, new untested features and more.

Enjoy!

EOF

cat upload_date.txt

# Set the IFS to newline to split the variable by lines
IFS=$'\n'

# Convert the variable into an array
file_array=($files)

upload_file upload_date.txt README.md

# Iterate over the array of filenames
for src in "${file_array[@]}"; do
    dest=`echo $src | sed -e "s/v$mrv2_VERSION/beta/"`
    upload_file $src $dest
done

# Reset IFS to its default value (space, tab, newline)
IFS=$' \t\n'

rm upload_date.txt

cd -
