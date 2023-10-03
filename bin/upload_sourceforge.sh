#!/usr/bin/env bash

echo "RUNNING upload_sourceforge.sh......"

. etc/functions.sh

# if [[ "$(git rev-parse --abbrev-ref HEAD)" != "main" ]]; then
#     echo "You are not on the main branch.  Will not make a release."
#     exit 0
# fi

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

get_kernel
# Extract cmake/version.cmake into mrv2_VERSION
extract_version
echo "mrv2 VERSION=$mrv2_VERSION"

#
# Go to packages directory
#
mkdir -p packages
cd packages

# Read all the files of this version
shopt -s nullglob
files=(*v${mrv2_VERSION}*)
shopt -u nullglob

if [[ "$files" == "" ]]; then
    echo
    echo "No v${mrv2_VERSION} files to upload.  Something probably went wrong in the compilation."
    echo
    exit 1
fi


if [[ $KERNEL == *Msys* ]]; then
    pacman -Sy openssh --noconfirm
    echo "ssh is:"
    which ssh
    echo "scp is:"
    which scp
fi


# Create the remote directory
ssh -i $SSH_KEY -o StrictHostKeyChecking=no ggarra13@frs.sourceforge.net 'mkdir -p /home/frs/project/mrv2/beta/'


upload_file()
{
    # Upload the file to the created directory
    echo
    echo "Sending $1 as $2..."
    echo
    scp -i $SSH_KEY $1 ggarra13@frs.sourceforge.net:/home/frs/project/mrv2/beta/$2
}



#
# Get the date in English
#
export oldTZ=$TZ
export oldTIME=$LC_TIME
export oldALL=$LC_ALL
export LC_ALL=en_US.UTF-8
export LC_TIME=en_US.UTF-8

#
# Report the date for 
#
export TZ="America/Argentina/Buenos_Aires"
date=`date "+%e of %B of %Y at %H:%M:%S Buenos Aires, Argentina (%z UTC/GMT)" | awk '{sub(/^ */, "", $1); print}'`
git_hash=`git rev-parse HEAD`
export TZ=$oldTZ
export LC_TIME=$oldTIME
export LC_ALL=$oldALL

#
# Create the README.md file
#
cat <<EOF > upload_date.md

mrv2 v${mrv2_VERSION} Beta
================

This beta release of v${mrv2_VERSION} corresponds to:
${date}.

It may contain bugs, new untested features and more.

It should work on Ubuntu 20.04+, Windows 8.1+ and macOS 11+.

This is unlike the official releases that work on Red Hat 8.1
and macOS 10.5 (Catalina).  Also, the Windows release does not
have USD support.

Its source code is at:
https://www.github.com/ggarra13/mrv2

Hash is ${git_hash}

You can extract it to compile yourself with:

\`\`\`
   git clone https://www.github.com/ggarra13/mrv2.git
   cd mrv2
   git checkout ${git_hash}
   ./runme.sh
\`\`\`


Enjoy!

----------------------------------------------------------------

EOF


HISTORY=../mrv2/docs/HISTORY.md

# Find the line number of "v${mrv2_VERSION}" in the file
start_line=$(grep -n "v${mrv2_VERSION}" "$HISTORY" | cut -d':' -f1)

# Find the line numbers of all "v0.*.*" occurrences in the file
end_lines=$(grep -n "v0\.[0-9]\+\.[0-9]\+" "$HISTORY" | cut -d':' -f1)

# Convert the end_lines string into an array
IFS=$'\n' read -rd '' -a end_lines <<< "$end_lines"

# Initialize the end_line to 0
end_line=${end_lines[1]}
end_line=$((end_line - 1))
start_line=$((start_line + 2))

# Use sed to extract the text between the two lines and store it in a variable
release_notes=$(sed -n "$start_line,${end_line}p" "$HISTORY")



echo "$release_notes" >> upload_date.md
cat upload_date.md

upload_file upload_date.md README.md
rm upload_date.md

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


