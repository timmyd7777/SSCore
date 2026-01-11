# CMake Build

I have been trying to standardize on 1 build platform for all the code I use/look at/look into - and so, when I found SSCore, I wanted to use CMake, but was surprised this was not possible. 

So this is my attempt at a CMake compliet version.

Despite this being developed on a Mac, it looks OS agnostic - which was my main intention.

# Build

If you are expierienced developer - this is a standard CMake build process. 
Currently there is no **install** phase. So you will have to move/manupulate the executables yourself.


## Detailed small steps


You need the git repo 

    #cd ~/Dev/Astro ? 
    git clone git@github.com:timseed/SSCore.git 

And now we enter the *source code* directory

    cd SSCore 

We just need to create a directory 

    mkdir build
    cd build
    cmake ..
    make 

That should be it!!

## If it fails 

There are some **assumed** software components 

  -- ZLib (A Library for compression. This may need installing, but it is a common library)
  -- C++ Compiler 
  -- Threads library installed (almost certainly you have this)
  -- Maths library installed (almost cdertainly you have this also)

If you renamed the Folder from **SSCore** to *sscore* - the CMakeLists.txt will not find the source files, equally if you have renamed/changed the case of any files. 


