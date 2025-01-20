# Run with: python3 build.py
import os
import platform

# (1)==================== COMMON CONFIGURATION OPTIONS ======================= #
COMPILER="g++ -g -std=c++17"   # The compiler
SOURCE="./src/*.cpp"    # Where the source code lives
EXECUTABLE="prog"        # Name of the final executable
# ======================= COMMON CONFIGURATION OPTIONS ======================= #

# (2)=================== Platform specific configuration ===================== #
ARGUMENTS=""            # Arguments needed for our program
INCLUDE_DIR=""          # Which directories
LIBRARIES=""            # What libraries

if platform.system()=="Linux":
    ARGUMENTS="-D LINUX"
    INCLUDE_DIR="-I ./include/ -I ./../common/thirdparty/glm/"
    LIBRARIES="-lSDL2 -ldl"
elif platform.system()=="Darwin":
    ARGUMENTS="-D MAC"
    INCLUDE_DIR="-I ./include/ -I/Library/Frameworks/SDL2.framework/Headers -I./../common/thirdparty/old/glm"
    LIBRARIES="-F/Library/Frameworks -framework SDL2"
elif platform.system()=="Windows":
    ARGUMENTS="-D MINGW -static-libgcc -static-libstdc++" 
    INCLUDE_DIR="-I./include/ -I./../common/thirdparty/old/glm/"
    EXECUTABLE="prog.exe"
    LIBRARIES="-lmingw32 -lSDL2main -lSDL2 -mwindows"
# ====================== Platform specific configuration ===================== #

# (3)====================== Building the Executable ========================== #
compileString=COMPILER+" "+ARGUMENTS+" "+SOURCE+" -o "+EXECUTABLE+" "+" "+INCLUDE_DIR+" "+LIBRARIES
print("===============================================================================")
print("====================== Compiling on: "+platform.system()+" =============================")
print("===============================================================================")
exit_code = os.system(compileString)
exit(0 if exit_code==0 else 1)
# ========================= Building the Executable ========================== #
