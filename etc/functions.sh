#
# Some auxiliary functions
#

#
# Simple function to run a command and print it out
#
run_cmd()
{
    echo
    echo "> $*"
    echo
    eval command $*
}
