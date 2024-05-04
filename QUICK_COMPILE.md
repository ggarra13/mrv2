
1. Create a bin/ directory in $HOME (a mrv2 symlink will be placed there).

   mkdir -p ${HOME}/bin

3. Clone the mrv2 repository.

   ```
   git clone https://github.com/ggarra13/mrv2.git --branch wayland_manolo --depth 1
   cd mrv2
   ```
   
4. Adjust the FLTK repository to point to your own git@..., so you can later
   commit your changes:

   ```
   nano cmake/Modules/BuildFLTK.cmake
   ```

5. Run:

   ```
   sudo apt install ninja-build
   ```

6. Compile a small binary of mrv2 (15 mins. compile on 16 CPUs):

   ```
   ./bin/runme_small.sh
   ```

   Wait for the build to finish.  Note that, at least from where I am,
   some very popular repositories at github.com sometimes fail cloning 3
   times, so you might need to run the script several times until it succeeds.

   If the build fails, you can find the compile log in:

      	BUILD-Linux-amd64/Release/compile.log
	
   to search for these keywords:
      	
	Failed to clone           # GitHub failing (re-run script)
	FAILED:                   # A build step failed (let me know)
	CMake Error:              # A cmake configuration error (let me know)

7. Download a movie that hangs for me to test with:

   I have uploaded one for you test (you will need Mb).

   Go to:


8. For your coding and quick testing, go to FLTK's repository in:

       BUILD-Linux-amd64/Release/FLTK-prefix/src/FLTK

   and hack at it.

9. Run a quick build (seconds to compile):

   ```
   ./runmet.sh
   ```

10. Additional notes.  All runme* scripts support a debug and -h parameter,
    like:

   ```
   ./bin/runme_small.sh debug
   ```

   albeit a debug build might mask the timeout error I think.
