#!/usr/bin/env bash

echo "RUNNING upload_sourceforge.sh......"

. etc/functions.sh

if [[ "$(git rev-parse --abbrev-ref HEAD)" != "main" ]]; then
    echo "You are not on the main branch.  Will not make a release."
    exit 0
fi

if [[ -e ssh/id_rsa ]]; then
    SSH_KEY=$PWD/ssh/id_rsa
else
    if [[ $HOME/.ssh/id256_rsa ]]; then
	SSH_KEY=$HOME/.ssh/id256_rsa
    elif [[ $HOME/.ssh/id_rsa ]]; then
	SSH_KEY=$HOME/.ssh/id_rsa
    else
	echo "Unknown SSH key file. Aborting..."
	exit 1
    fi
fi
echo "SSH KEY IS: ${SSH_KEY}"


# Create the remote directory
ssh -i $SSH_KEY -o StrictHostKeyChecking=no ggarra13@frs.sourceforge.net 'mkdir -p /home/frs/project/mrv2/beta/'


upload_file()
{
    # Upload the file to the created directory
    echo
    echo "Sending $1 as $2..."
    echo
    rsync -avP --ignore-errors -e "ssh -i $SSH_KEY" $1 ggarra13@frs.sourceforge.net:/home/frs/project/mrv2/beta/$2
}

# Extract cmake/version.cmake into mrv2_VERSION
extract_version

echo "mrv2 VERSION=$mrv2_VERSION"


#
# Get the date in English
#
export LC_ALL=en_US.UTF-8
export LC_TIME=en_US.UTF-8
date=`date "+%e of %B of %Y at %H:%M:%S" | awk '{sub(/^ /, "", $1); print}'`
git_hash=`git rev-parse HEAD`

#
# Go to packages directory
#
cd packages

#
# Create the README.md file
#
cat <<EOF > upload_date.md

mrv2 Beta
=========

This beta release of v${mrv2_VERSION} corresponds to ${date}.

It may contain bugs, new untested features and more.

It should work on Ubuntu 20.04+, Windows 8.1+ and macOS 11+.

This is unlike the official releases that work on Red Hat 8.1
and macOS 10.5 (Catalina).

It corresponds to https://www.github.com/ggarra13/mrv2

Hash is ${git_hash}

You can extract it to compile yourself with:

\`\`\`
   git clone https://www.github.com/ggarra13/mrv2.git
   cd mrv2
   git checkout ${git_hash}
   ./runme.sh
\`\`\`


Enjoy!

EOF

upload_file upload_date.md README.md
rm upload_date.md


# Read all the files of this version
files=`ls -1 *v${mrv2_VERSION}*`

# Set the IFS to newline to split the variable by lines
IFS=$'\n'

# Convert the variable into an array
file_array=($files)

# Iterate over the array of filenames
for src in "${file_array[@]}"; do
    dest=`echo $src | sed -e "s/v$mrv2_VERSION/beta/"`
    upload_file $src $dest
done

# Reset IFS to its default value (space, tab, newline)
IFS=$' \t\n'


# Go back to root directory
cd ..

