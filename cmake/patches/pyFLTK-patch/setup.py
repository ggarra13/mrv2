from setuptools import setup
from distutils.core import setup, Extension
from distutils.sysconfig import get_python_version
import distutils.cmd
import distutils.log
import subprocess
import os, sys, string, platform, shutil, glob

# these settings will read setup information from the environment.
# instead of this, the relevant paths can be set directly here:
#
fltk_dir = ""
opengl_dir = ""
# this will be overrridden by environment variables:
fltk_dir = os.environ.get('FLTK_HOME', '')
opengl_dir = os.environ.get('OPENGL_HOME', '')

# Use this to pass additional compile flags, like macOS -mmacosx-version-min=OS_VERSION
cxx_flags = os.environ.get('CXX_FLAGS', '')
print("pyFLTK CXX_FLAGS=",cxx_flags)

# add your extensions here
UserDefinedSources = []
# add additional include paths here
UserIncludeDirs = []

# do not edit beyond this point
###########################################################################
# command line configurations
# --enable-shared   (for Windows distributions that ship a single DLL)
# --disable-gl
# --disable-forms
##########################################################################
doCheckForms = True
doCheckGl = True
isVerbose = True

doMulti = False
doOpenGL = False
doForms = False
doShared = False

# debug flag
doDebug = False
new_args = []
for item in sys.argv:
    if item == '--disable-gl':
        doCheckGl = False
        if isVerbose:
            print("No OpenGL support!")
    elif item == '--disable-forms':
        doCheckForms = False
        if isVerbose:
            print("No Forms support")
    elif item == '--debug':
        doDebug = True
        if isVerbose:
            print("Detected DEBUG build!")
    elif item == '--enable-shared':
        doShared = True
    else:
        new_args.append(item)
sys.argv = new_args

###########################################################################
#
# create proper paths
fltk_lib_dir = os.path.join(fltk_dir, 'lib')
fltk_includes = []
compile_arg_list = []
link_arg_list = []

# whatever is platform dependent
if sys.platform == 'win32':
    print("Building for MS Windows, using Visual C++")
    opengl_lib_dir = os.path.join(opengl_dir, 'lib')
    def_list = [('WIN32', '1'),('FL_INTERNALS','1')]
    compile_arg_list= ['/GR', '/wd4101']
    lib_dir_list = [fltk_lib_dir, opengl_lib_dir]
    win32_lib_list = ["kernel32", "user32", "gdi32", "winspool", "comdlg32", "Comctl32", "advapi32", "shell32", "ole32", "oleaut32", "uuid", "odbc32", "odbccp32", "wsock32", "gdiplus", "glu32", "opengl32"]
    static_lib_list = [ "fltk", "fltk_images", "fltk_forms", "fltk_gl", "opengl32", "fltk_jpeg", "fltk_png", "fltk_z"] + win32_lib_list
    shared_lib_list = ["libfltk", "fltk_jpeg", "fltk_png", "fltk_z"] + win32_lib_list
    if doShared:
        def_list = def_list + [('FL_DLL', 1)]
        lib_list = shared_lib_list
    else:
        lib_list = static_lib_list
elif sys.platform.startswith('linux')  or sys.platform.startswith('gnukfreebsd') or sys.platform.startswith('gnu0'):
    print("Building for Linux")

    def_list = [('UNIX', '1'),('FL_INTERNALS','1')]
    compile_arg_list.append('-Wno-unused-label')
    compile_arg_list.append('-Wno-unused-but-set-variable')
    compile_arg_list.append('-Wformat=1')
    compile_arg_list.append('-Werror=format-security')
    lib_dir_list = [fltk_lib_dir, '/usr/lib']
    lib_list = ["fltk"]
elif sys.platform in ['freebsd4','freebsd5','freebsd6','freebsd7', 'sunos5']:
    print(f"Building for: {sys.platform}")
    def_list = [('UNIX', '1')]
    lib_dir_list = [fltk_lib_dir,'/usr/X11R6/lib','/usr/lib']
    lib_list = ["fltk"]
elif sys.platform == 'darwin':
    print("Building for  Mac OS X")
    def_list = [('UNIX', '1'),('FL_INTERNALS','1')]
    lib_dir_list = [fltk_lib_dir]
    lib_list = ["fltk"]
    cpu_type = platform.processor()
    if cpu_type.startswith("i386"):
        print("i386 CPU variant detected")
        lib_dir_list.append('/usr/local/lib')
        osx_arch = "x86_64"
    elif cpu_type.startswith("arm"):
        print("arm CPU variant detected")
        lib_dir_list.append('/opt/homebrew/lib')
        osx_arch = "arm64"
    else:
        print("PowerPC system detected")
        osx_arch = "ppc"

    compile_arg_list=['-arch', osx_arch]
    link_arg_list=['-stdlib=libc++', '-arch', osx_arch, '-framework','ApplicationServices','-framework','Carbon','-framework', 'Cocoa', '-framework','OpenGL','-framework','AGL']

else:
    print("Platform not officially supported!")
    print("You can try to edit the platform specific settings in the file setup.py by creating an entry for the following platform: ", sys.platform)
    sys.exit(0)

    

###########################################################################
# test for fltk configuration (libraries)
def fltk_config(dir):
    global doMulti
    "return library paths and additional libraries that were used to link FLTK"
    needed_libraries = []
    needed_directories = []
    needed_includes = []

    ver_cmd = None
    inc_cmd = None
    lib_cmd = None
    # always use images
    var_string = " --use-images"
    if doCheckGl:
        var_string = var_string + " --use-gl --use-glut"
    if doCheckForms:
        var_string = var_string + " --use-forms"
    try:
        if isVerbose:
            print("Checking fltk-config using FLTK_HOME")
        fltk_dir = os.environ['FLTK_HOME']
        ver_cmd = f"sh {fltk_dir}/fltk-config --version"
        inc_cmd = f"sh {fltk_dir}/fltk-config --cxxflags {var_string}"
        #lib_cmd = f"sh {fltk_dir}/fltk-config --use-gl --use-glut --use-images --use-forms --ldflags"
        lib_cmd = f"sh {fltk_dir}/fltk-config --ldflags {var_string}"
    except:
        if isVerbose:
            print("Checking fltk-config using default installation")

        ver_cmd = "fltk-config --version"
        inc_cmd = f"fltk-config --cxxflags {var_string}"
        lib_cmd = f"fltk-config --ldflags {var_string}"

    # version
    result = os.popen(ver_cmd).readlines()
    if len(result) == 0:
        print("No version information for FLTK found!")
    else:
        print("Using FLTK: ", result)
        
    # include flags
    result = os.popen(inc_cmd).readlines()
    if len(result) == 0:
        print("No compile flags found!")
    else:
        inc_list = map(lambda x: x.strip(), result[0].split(' '))
#        inc_list = [x.strip() for x in result[0].split(' ')]
    
        for inc in inc_list:
            if inc[:2] == '-I':
                needed_includes.append(inc[2:])
            if inc.find("_REENTRANT") >= 0:
                doMulti = True
        print("fltk-config includes: ", needed_includes)

    # lib flags
    result = os.popen(lib_cmd).readlines()
    if len(result) == 0:
        print("No link flags found!")
    else:
        lib_list = map(lambda x: x.strip(), result[0].split(' '))
#        lib_list = [x.strip() for x in result[0].split(' ')]
    
        for lib in lib_list:
            if lib[:2] == '-l':
                needed_libraries.append(lib[2:])
            if lib[:2] == '-L':
                needed_directories.append(lib[2:])
        print("fltk-config link paths: ", needed_directories)
        print("fltk-config link libraries: ", needed_libraries)
                
    return (needed_libraries, needed_directories, needed_includes)

###########################################################################
all_include_dirs = ['./src', './contrib','/usr/include']
if fltk_dir != "":
    if (sys.platform == 'win32'):
        all_include_dirs.insert(0, fltk_dir+"/include")
    else:
        all_include_dirs.insert(0, fltk_dir)
print(all_include_dirs)
###########################################################################

if not (sys.platform == 'win32'):
    print("Checking FLTK configuration ... ")
    additional_libs, additional_dirs, additional_includes = fltk_config(fltk_dir)
    
    for item in additional_includes:
        # already included?
        add = True
        for used_inc in all_include_dirs:
            if item == used_inc:
                add = False
                break
        if add:
            all_include_dirs.insert(0, item)

    # check also for multi-threading
    pos = 0
    for item in additional_libs:
        lowercase_item = item.lower()
        if lowercase_item.find("pthread") >= 0:
            doMulti = True
        if lowercase_item.find("fltk") >= 0 and lowercase_item.find("gl") >= 0:
            doOpenGL = True
        if lowercase_item.find("fltk") >= 0 and lowercase_item.find("forms") >= 0:
            doForms = True
            
    # simply add all the libraries to the front of the used libraries
    lib_list = additional_libs+lib_list
    
    if not doMulti:
        # disable multi-threading support
        print("FLTK was configured without multi-threading support!")
        compile_arg_list.append("-DDO_NOT_USE_THREADS")
    else:
        print("FLTK was configured with multi-threading support!")

    # On Mac OSX, openGL is always used for FLTK
    if sys.platform == 'darwin':
        doOpenGL = True

    if not doOpenGL:
        # disable OpenGL support
        print("FLTK was configured without OpenGL support!")
        UserDefinedSources.append('./src/Fl_Gl_Stubs.cxx')
        compile_arg_list.append("-DDO_NOT_USE_OPENGL")
    else:
        print("FLTK was configured with OpenGL support!")

    if not doForms:
        # disable Forms support
        print("FLTK was configured without Forms support!")
        UserDefinedSources.append('./src/Fl_Forms_Stubs.cxx')
    else:
        print("FLTK was configured with Forms support!")

    # add all the library paths
    lib_dir_list = additional_dirs+lib_dir_list

    print("done")

###########################################################################

class PySwigCommand(distutils.cmd.Command):
  """A custom command to run swig on the interface files."""
  description = 'run swig on the interface files'
  user_options = [
      # The format is (long option, short option, description).
      #('pylint-rcfile=', None, 'path to Pylint config file'),
  ]
  include = []

  def initialize_options(self):
    """Set default values for options."""
    # Each user option must be listed here with their default value.
    self.pylint_rcfile = ''

  def finalize_options(self):
    """Post-process options."""
    add_incl = ['-I/usr/include']
    try:
        fltk_dir = os.environ['FLTK_HOME']
        if (sys.platform == 'win32'):
            add_incl.insert(0, f"-I{fltk_dir}/include")
        else:
            add_incl.insert(0, f"-I{fltk_dir}")
    except:
        print("Using default location for FLTK!")
        result = os.popen('fltk-config --cxxflags').readlines()
        #print(result)
        if len(result) > 0:
            p_inc = map(lambda x: x.strip(), result[0].split(' '))
#            p_inc = [x.strip() for x in result[0].split(' ')]
            for item in p_inc:
                #if string.find(item, '-I') == 0:
                if item.find('-I') == 0:
                    add_incl.insert(0, item)
        else:
            print("FLTK not found!")
    self.include = add_incl

  def run(self):
    """Run command."""
    command = ['swig', '-D{0}'.format(sys.platform.upper()), '-DFL_INTERNALS', '-w302', '-w312', '-w325', '-w362', '-w389', '-w401', '-w473', '-w509', '-I./swig', '-DPYTHON', '-DPYTHON3', '-c++', '-python', '-shadow', '-fastdispatch', '-outdir', 'fltk14', '-o', 'fltk14/fltk_wrap.cpp', './swig/fltk.i']
    pos = command.index('-I./swig')
    if sys.platform.upper() == 'DARWIN':
        command[pos:pos] = ["-D__APPLE__"]
        pos += 2
    else:
        pos +=1
    command[pos:pos] = self.include
    self.announce(
        'Running command: %s' % str(command),
        level=distutils.log.INFO)
    #subprocess.check_call(command, cwd='python')
    subprocess.check_call(command)

    #shutil.copy('fltk.py', "./fltk/fltk.py")

if cxx_flags != '':
    compile_arg_list.append(cxx_flags)

# module declarations
module1 = Extension(name='fltk14._fltk14',
		    define_macros=def_list,
		    include_dirs = all_include_dirs+UserIncludeDirs,
                    sources = ['./fltk14/fltk_wrap.cpp',
                               './contrib/ListSelect.cpp']+UserDefinedSources,
		    extra_compile_args=compile_arg_list,
                    extra_link_args=link_arg_list,
		    library_dirs=lib_dir_list,
          	    libraries=lib_list)

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setup (cmdclass={
        'swig': PySwigCommand,
       },
       name = 'pyFltk1.4',
       version = '1.4.0rc1',
       #setup_requires=['wheel'],
       ext_modules = [module1],
       packages = ['fltk14','fltk14.test'],

       package_data={'fltk14': ['fltk14']},
       include_package_data=True,

       # metadata to display on PyPI
       author = 'Andreas Held',
       author_email = 'andreasheld@users.sourceforge.net',
       url = 'http://pyfltk.sourceforge.net',
       description = 'This is a Python wrapper for the FLTK',
       long_description=long_description,
       long_description_content_type="text/markdown",
       keywords="python fltk",
       project_urls={
           "Bug Tracker": "https://sourceforge.net/p/pyfltk/bugs/",
           "Documentation": "https://fltk.gitlab.io/fltk/",
           "Source Code": "https://sourceforge.net/p/pyfltk/code/HEAD/tree/branches/fltk1.4/",
       },
       classifiers=[
           "License :: OSI Approved :: GNU Lesser General Public License v2 (LGPLv2)"
       ]
)



