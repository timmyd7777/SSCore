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

# Testing 

When the **make** process has finished, there should be 4 executable files in the *build* folder.

The easiest way to test things have worked is top use the make custom run commands 

    make run 
    make runmount 
    make runtetra 
    make runtle 

Lets do this in that order.

I am assuming 

  - project is built
  - you are in the build folder 

## make run 

Just use the following command 

    make run 

I see lots of data on the screen - it ends with 

```text
Imported 15670 non-Earth planetary surface features
Indexed 45 planets with surface features
Imported 4444 cities
Imported 943 MPC comets
Imported 13798 MPC asteroids
Exported 290 natural satellites to ./ExportedMoons.csv
Exported 15670 non-Earth planetary surface features to ./ExportedFeatures.csv
Exported 4444 cities to ./ExportedCities.csv
Exported 943 MPC comets to ./ExportedComets.csv
Exported 13798 MPC asteroids to ./ExportedAsteroids.csv
Imported 88 IAU constellations
Imported 1562 IAU constellation boundary vertices
Imported 765 IAU constellation shape lines
Exported 88 constellations to ./ExportedConstellations.csv
Imported 377 nearby stars
Imported 9092 bright stars
Exported 377 nearby stars to ./ExportedNearbyStars.csv
Exported 9092 bright stars to ./ExportedBrightStars.csv
Imported 109 Messier objects
Imported 112 Caldwell objects
Exported 109 Messier objects to ./ExportedMessier.csv
Exported 112 Caldwell objects to ./ExportedCaldwell.csv
```

If you check the build folder you will see lots of csv files which you can open in your Spreadsheet (Numbers, Excel, OpenOffice etc) software.

## make runmount 

Just use the following command 

    make runmount

Again I see lots of numbers on the screen - with the ending line of 

```text
RMS alignment error: 1.3236 degrees
```

I assume this has worked !!

## make runtetra

Just use the command 

    make runtetra 

I see the following output 

```text
Loaded Tetra3 database with 116537 patterns and 6043 stars
Solved 66 sources in 1.20358 ms.
R.A.: 15 41 55.40
Dec.: +30 04 57.5
FoV:  23.6912 deg
Roll: 289.211 deg
```

## make runtle

You guessed it, you type 

    make runtle 

I get masses of screen output - but it ends 

```text 
1438.0 +4818.937 -1794.878 -4373.072 -0.236108 +7.005459 -3.140045  6750.370  7.680630
1439.0 +4793.565 -1370.700 -4551.120 -0.609285 +7.128354 -2.792581  6750.539  7.680050
1440.0 +4745.882  -940.142 -4707.924 -0.979510 +7.218038 -2.432193  6750.689  7.679523
```

I again make the assumption it all works.


# Next Steps 

As this code provided us with a static library, **libsscore.a**, we could look into using this - for our own plate solving routines.
