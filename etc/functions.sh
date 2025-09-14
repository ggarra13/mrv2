#!/usr/bin/env bash
# SPDX-License-Identifier: CC BY-NC-ND
# Filmmaker
# Copyright 2024 - Gonzalo Garramuño. All rights reserved.


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
# Get the linux id if available
#
get_linux_id()
{
    export LINUX_ID=`cat /etc/os-release | grep 'ID_LIKE='`
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
	export NATIVE_C_COMPILER=`which cl.exe`
	export NATIVE_C_COMPILER_NAME="cl.exe"
	export NATIVE_CXX_COMPILER=`which cl.exe`
	export NATIVE_CXX_COMPILER_NAME="cl.exe"

	if [[ "$GENERIC_C_COMPILER" == "" ]]; then
	    export GENERIC_C_COMPILER=`which clang.exe`
	    export GENERIC_C_COMPILER_NAME="clang.exe"
	fi
	if [[ "$GENERIC_CXX_COMPILER" == "" ]]; then
	    export GENERIC_CXX_COMPILER=`which clang.exe`
	    export GENERIC_CXX_COMPILER_NAME="clang.exe"
	    if [[ "$GENERIC_CXX_COMPILER" == "" ]]; then
		echo "WARNING: GENERIC_CXX_COMPILER for "\
		     "this platform was not found"
	    fi
	fi
	
    elif [[ $KERNEL == *Darwin* ]]; then
	export MACOS_BRAND=$(sysctl -n machdep.cpu.brand_string)
	export NATIVE_CXX_COMPILER=`which clang`
	export NATIVE_CXX_COMPILER_NAME="clang"
	export GENERIC_CXX_COMPILER=`which cc`
	export GENERIC_CXX_COMPILER_NAME="cc"
    else
	get_linux_id()

	if [[ $LINUX_ID == *debian* ]]; then
	    export NATIVE_C_COMPILER=`which gcc`
	    export NATIVE_C_COMPILER_NAME="gcc"
	    export NATIVE_CXX_COMPILER=`which g++`
	    export NATIVE_CXX_COMPILER_NAME="g++"
	elif [[ $LINUX_ID == *rhel* ]]; then
	    export NATIVE_C_COMPILER=`which cc`
	    export NATIVE_C_COMPILER_NAME="cc"
	    export NATIVE_CXX_COMPILER=`which c++`
	    export NATIVE_CXX_COMPILER_NAME="c++"
	fi

	if [[ "$NATIVE_C_COMPILER" == "" ]]; then
	    export NATIVE_C_COMPILER=`which gcc`
	    export NATIVE_C_COMPILER_NAME="gcc"
	fi
	if [[ "$NATIVE_CXX_COMPILER" == "" ]]; then
	    export NATIVE_CXX_COMPILER=`which g++`
	    export NATIVE_CXX_COMPILER_NAME="g++"
	fi
	if [[ "$GENERIC_C_COMPILER" == "" ]]; then
	    export GENERIC_C_COMPILER=`which gcc`
	    export GENERIC_C_COMPILER_NAME="gcc"
	fi
	if [[ "$GENERIC_CXX_COMPILER" == "" ]]; then
	    export GENERIC_CXX_COMPILER=`which g++`
	    export GENERIC_CXX_COMPILER_NAME="g++"
	fi
    fi

    if [[ $ARCH == "" ]]; then
	export ARCH=`uname -m` # was uname -a
	export UNAME_ARCH=$ARCH # Store uname architecture to compile properly
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
	export NATIVE_COMPILER_VERSION="MSVC ${MSVC_VERSION}"
	export GENERIC_COMPILER_VERSION=`clang --version | grep version`
    elif [[ $KERNEL == *Linux* ]]; then
	export NATIVE_COMPILER_VERSION=`gcc --version | grep gcc`
    else
	export NATIVE_COMPILER_VERSION=`clang --version | grep version`
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
# PYTHON_SITEDIR      - directory of python site libraries (site-packages)
# PYTHON_USER_SITEDIR - directory of user's pythons site libraries 
# PYTHON_LIBDIR       - directory of python dynamic libraries
#

locate_python() {
    # Clear previous exports to ensure a clean slate
    unset PYTHONDIR PYTHONEXE PYTHON PYTHON_VERSION PYTHON_SITEDIR PYTHON_USER_SITEDIR PYTHON_LIBDIR

    local executables=("python" "python3" "python3.11" "python3.10" "python3.9")
    local locations
    
    # Check if BUILD_DIR exists and is a directory
    if [[ -d "${BUILD_DIR}/install/bin" ]]; then
        locations="${BUILD_DIR}/install/bin /usr/local/bin ${PATH} /usr/bin"
    else
        locations="/usr/local/bin ${PATH} /usr/bin"
    fi

    # Loop through locations and executables to find a working Python
    for loc in $(echo "${locations}" | tr ':' ' '); do
        for exe in "${executables[@]}"; do
            local full_path="$(command -v "${loc}/${exe}" 2>/dev/null)"
            if [[ -x "${full_path}" ]]; then
                export PYTHON="${full_path}"
                export PYTHONEXE="${exe}"
                export PYTHONDIR="$(dirname "${full_path}")"
                break 2 # Found it, exit both loops
            fi
        done
    done

    # If Python is still not found, handle the error
    if [[ -z "${PYTHON}" ]]; then
        if [[ -z $BUILD_PYTHON ]]; then
            echo "No python found! Please install it in your PATH." >&2
            exit 1
        fi        # Fallback for when BUILD_PYTHON is set
        export PYTHONDIR="${PWD}/${BUILD_DIR}/install/bin/"
        if [[ "${KERNEL}" != *Msys* && "${KERNEL}" != *MSYS* && "${KERNEL}" != *mingw* ]]; then
            export PYTHONEXE=python3
        else
            export PYTHONEXE=python
        fi
        export PYTHON="${PYTHONDIR}/${PYTHONEXE}"
    fi
    # Normalize paths for Windows (convert to Unix-style if in Msys/Cygwin)
    if [[ "${KERNEL}" == *Msys* || "${KERNEL}" == *MSYS* || "${KERNEL}" == *mingw* ]]; then
        PYTHON=$(cygpath -u "${PYTHON}" 2>/dev/null || echo "${PYTHON}")
        PYTHONDIR=$(cygpath -u "${PYTHONDIR}" 2>/dev/null || echo "${PYTHONDIR}")
    fi
    
    # Use Python to determine version and directories
    if [[ -x "${PYTHON}" ]]; then
        local python_info
	python_info=$("${PYTHON}" -c "import sys; import site; import os; print(f'VER={sys.version_info.major}.{sys.version_info.minor}\nLIBDIR={sys.exec_prefix}/lib\nUSER_SITEDIR={site.getusersitepackages()}'); sp_dirs=site.getsitepackages(); sitedir=[d for d in sp_dirs if d.endswith('site-packages')][0] if [d for d in sp_dirs if d.endswith('site-packages')] else ''; print(f'SYSTEM_SITEDIR={sitedir}')" 2>/dev/null)
        if [[ -n "${python_info}" ]]; then
            export PYTHON_VERSION=$(echo "${python_info}" | grep 'VER=' | cut -d'=' -f2)
            export PYTHON_LIBDIR=$(echo "${python_info}" | grep 'LIBDIR=' | cut -d'=' -f2)
            export PYTHON_SITEDIR=$(echo "${python_info}" | grep 'SYSTEM_SITEDIR=' | cut -d'=' -f2)
            export PYTHON_USER_SITEDIR=$(echo "${python_info}" | grep 'USER_SITEDIR=' | cut -d'=' -f2)
        else
            echo "Could not get Python details from ${PYTHON}. Falling back to old method." >&2
            extract_python_version # Your fallback function
        fi
    fi

    # Report if crucial variables are missing
    if [[ ! -d "${PYTHON_LIBDIR}" ]]; then
        echo "Python libdir could not be determined! PYTHON_LIBDIR=${PYTHON_LIBDIR}" >&2
    fi
    if [[ ! -d "${PYTHON_SITEDIR}" ]]; then
        echo "Python site-packages could not be determined! PYTHON_SITEDIR=${PYTHON_SITEDIR}" >&2
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
	package_dir=$PWD/packages/$BUILD_DIR
	mkdir -p $package_dir
	if [[ -e $package ]]; then
	    echo "Installing $package in $package_dir"
	    run_cmd mv $package $package_dir
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
    
