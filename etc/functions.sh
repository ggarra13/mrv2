#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# Some auxiliary functions
#

#
# Simple function to run a command and print it out
#
run_cmd()
{
    echo "> $@"
    # These quick commands we won't time them
    if [[ "$1" == "rm" || "$1" == "mv" || "$1" == "cp" || \
	      "$1" == "ln" ]]; then
	eval command "$@"
    else
	time eval command "$@"
	echo
    fi
}

#
# Get kernel and architecture and on MacOS, MACOS_BRAND (Intel, M1, M2, etc).
#
get_kernel()
{
    export KERNEL=`uname`
    export MACOS_BRAND=''
    if [[ $KERNEL == *MSYS* || $KERNEL == *MINGW* ]]; then
	export KERNEL=Msys
	export ARCH=`which cl.exe`
    elif [[ $KERNEL == *Darwin* ]]; then
	    export MACOS_BRAND=$(sysctl -n machdep.cpu.brand_string)
    fi

    if [[ $ARCH == "" ]]; then
	export ARCH=`uname -m` # was uname -a
    fi

    if [[ $ARCH == arm64 ]]; then
	export ARCH=arm64
    elif [[ $ARCH == aarch64 ]]; then
	export ARCH=aarch64
    elif [[ $ARCH == *64* ]]; then
	export ARCH=amd64
    else
	export ARCH=i386
    fi
}

get_msvc_version()
{
    export MSVC_VERSION=`echo $VCINSTALLDIR | grep -o '2[0-9]\+'`
}

get_cmake_version()
{
    export CMAKE_LOCATION=`which cmake`
    export CMAKE_VERSION=`cmake --version | grep -o 'cmake version [0-9.]*' | cut -d' ' -f3`
}

get_compiler_version()
{
    if [[ $KERNEL == *Msys* ]]; then
	get_msvc_version
	export COMPILER_VERSION="MSVC ${MSVC_VERSION}"
    elif [[ $KERNEL == *Linux* ]]; then
	export COMPILER_VERSION=`gcc --version | grep gcc`
    else
	export COMPILER_VERSION=`clang --version | grep version`
    fi
}


#
# Extract version from cmake/version.cmake
#
extract_version()
{
    local major=`cat cmake/version.cmake | grep -o 'VERSION_MAJOR\s*[0-9]' | sed -e 's/VERSION_MAJOR[ \t]*//'`
    local minor=`cat cmake/version.cmake | grep -o 'VERSION_MINOR\s*[0-9]' | sed -e 's/VERSION_MINOR[ \t] *//'`
    local patch=`cat cmake/version.cmake | grep -o 'VERSION_PATCH\s*[0-9]-*[a-z]*[A-Z]*-*[0-9]*' | sed -e 's/VERSION_PATCH[ \t]*//'`
    export mrv2_VERSION="${major}.${minor}.${patch}"
}

#
# Extract python version from cmake/Modules/BuildPython.cmake
#
extract_python_version()
{
    local major=`cat cmake/Modules/BuildPython.cmake | grep -o 'set.[ \t]*Python_VERSION\s*[0-9]*' | sed -e 's/set.[ \t]*Python_VERSION[ \t]*//'`
    local minor=`cat cmake/Modules/BuildPython.cmake | grep -o 'set.[ \t]*Python_VERSION\s*[0-9]*\.[0-9]*' | sed -e 's/set.[ \t]*Python_VERSION[ \t]*[0-9]*\.//'`
    export PYTHON_VERSION="${major}.${minor}"
}

#
# Function to locate python.  Returns:
# PYTHONDIR           - directory of python executable
# PYTHONEXE           - name of python executable
# PYTHON              - full path to python executable
# PYTHON_VERSION      - full version of python executable, like 3.11
# PYTHON_SITEDIR      - directory of python site libraries
# PYTHON_USER_SITEDIR - directory of user's pythons site libraries 
# PYTHON_LIBDIR       - directory of python dynamic libraries
#
locate_python()
{
    export PYTHONEXE=""
    local locations="${BUILD_DIR}/install/bin ${PATH} /usr/local/bin /usr/bin"
    if [[ $KERNEL == *Msys* ]]; then
	locations=`echo "${locations}" | sed -e 's/;/ /g'`
    else
	locations=`echo "${locations}" | sed -e 's/:/ /g'`
    fi

    for location in $locations; do
	export pythons=$(ls ${location}/python* 2> /dev/null) || PYTHON=""
	if [[ "$pythons" != "" ]]; then
	    pythons=`echo "$pythons" | sed -e 's#/python.sh##'`
	    export PYTHONDIR=$location
	    export PYTHONEXE=`echo "$pythons" | grep -o '/python.*' | head -1`
	    export PYTHON=$PYTHONDIR/$PYTHONEXE
	    if [[ $KERNEL != *Msys* ]]; then
		while true; do
		    if [[ -L $PYTHON ]]; then
			export PYTHONEXE=`readlink ${PYTHON}`
			export PYTHON=$PYTHONDIR/$PYTHONEXE
		    else
			break
		    fi
		done
	    fi
	    break
	fi
    done

    if [[ "$PYTHONEXE" == "" ]]; then
	echo "No python found!!! Please install it in your PATH"
    fi

    export PYTHON_LIBDIR="-unknown-"
    local lib_dirs="~/.local/lib/${PYTHONEXE} ${PYTHONDIR}/../lib/${PYTHONEXE} ${PYTHONDIR}/Lib"
    for python_libdir in $lib_dirs; do
	if [[ $KERNEL != *Msys* ]]; then
	    if [[ -d "${python_libdir}" ]]; then
		python_libdir=`readlink -f "${python_libdir}"`
	    fi
	fi
	if [[ -d "${python_libdir}" ]]; then
	    export PYTHON_LIBDIR="${python_libdir}"
	    break
	fi
    done
    
    local site_dirs="${PYTHON_LIBDIR}"
    export PYTHON_SITEDIR="-unknown-"
    export PYTHON_USER_SITEDIR="${HOME}/.local/lib/${PYTHONEXE}"
    for python_sitedir in $site_dirs; do
	if [[ -d "${python_sitedir}/site-packages" ]]; then
	    export PYTHON_SITEDIR="${python_sitedir}/site-packages"
	    break
	fi
    done

    if [[ ! -d "${PYTHON_LIBDIR}" ]]; then
	echo "Python libdir could not be determined!"
	echo "PYTHON_LIBDIR=${PYTHON_LIBDIR}"
    fi

    if [[ ! -d "${PYTHON_SITEDIR}" ]]; then
	echo "Python site-packages could not be determined!"
	echo "PYTHON_SITEDIR=${PYTHON_SITEDIR}"
    fi

    if [[ $KERNEL == *Msys* ]]; then
	local python_dlls=`ls ${PYTHONDIR}/python*.dll`
	local python_dll=`echo "$python_dlls" | grep -o '/python.*' | head -1`

	local full=`echo "${python_dll}" | sed -e 's#.*/python##' | sed -e 's#.dll##'`
	local major=`echo $full | sed -e 's#[0-9][0-9]$##'`
	local minor=`echo $full | sed -e 's#^[0-9]##'`
	export PYTHON_VERSION="${major}.${minor}"
    else
	export PYTHON_VERSION=`echo "${PYTHON_LIBDIR}" | sed -e 's#.*/python##'`
    fi

    #
    # If python version failed, get it from the cmake script.
    #
    if [[ "$PYTHON_VERSION" == "" || "$PYTHON_VERSION" == "." ]]; then
	extract_python_version
    fi
    
}


#
# Auxiliary function to send from the staging location to the packages/
#
send_to_packages()
{
    local stage=$PWD/$BUILD_DIR/mrv2/src/mrv2-build
    local package=$stage/$1
    if [[ "$CMAKE_TARGET" != "" ]]; then
	mkdir -p $PWD/packages
	if [[ -e $package ]]; then
	    echo "Installing $package in $PWD/packages"
	    run_cmd mv $package $PWD/packages
	else
	    echo "ERROR package $1 was not created in $stage."
	fi
    else
	echo "CMAKE_TARGET is empty.  Will not copy packages."
    fi
}


# Function to ask the question and return 1 for yes, 0 for no
# in response variable
ask_question()
{
    while true; do
	read -p "$1 (y/n): " answer
	case "$answer" in
	    [Yy]*)
		response=1; break;;
	    [Nn]*)
		response=2; break;;
	    *) echo "Please answer y or n." ;;
	esac
    done
}

#
# Auxiliary function to ask to continue (y/n)
#
ask_to_continue()
{
    echo ""
    echo "Are you sure you want to continue? (y/n)"
    read input
    if [[ $input == n* || $input == N* ]]; then
	exit 0
    fi
}

clean_mo_files()
{
    local lcdir
    local locales_dir
    echo ""
    echo "Cleaning .mo files"
    echo
    
    locales_dir="${BUILD_DIR}/install/share/locale/* ${BUILD_DIR}/install/python/plug-ins/locales/*"
    
}
    
