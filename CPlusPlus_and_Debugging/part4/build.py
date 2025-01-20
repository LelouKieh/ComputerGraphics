# Run with: python3 build.py
import os
import platform

# (1)==================== COMMON CONFIGURATION OPTIONS ======================= #
COMPILER="g++ -g -std=c++17"
SOURCE="./src/*.cpp"    # Where the source code is
EXECUTABLE="prog"        # Name of the final executable
# ======================= COMMON CONFIGURATION OPTIONS ======================= #

# (2)=================== Platform specific configuration ===================== #
ARGUMENTS=""            # Arguments needed for our program
INCLUDE_DIR=""          # Which directories do we want to include
LIBRARIES=""            # What libraries do we want to include

if platform.system()=="Linux":
    ARGUMENTS="-D LINUX" # -D is a #define sent to preprocessor
    INCLUDE_DIR="-I ./include/ -I ./../../common/thirdparty/glm/"
    LIBRARIES=""
elif platform.system()=="Darwin":
    ARGUMENTS="-D MAC" # -D is a #define sent to the preprocessor.
    INCLUDE_DIR="-I ./include/ -I/Library/Frameworks/SDL2.framework/Headers -I./../../common/thirdparty/old/glm"
    #LIBRARIES=""
elif platform.system()=="Windows":
    COMPILER="g++ -std=c++17"
    ARGUMENTS="-D MINGW -static-libgcc -static-libstdc++"
    INCLUDE_DIR="-I./include/ -I./../../common/thirdparty/old/glm/"
    EXECUTABLE="prog.exe"
    LIBRARIES="-lmingw32 -mwindows"

# (3)====================== Building the Executable ========================== #
# Build a string of our compile commands that we run in the terminal
compileString=COMPILER+" "+ARGUMENTS+" -o "+EXECUTABLE+" "+" "+INCLUDE_DIR+" "+SOURCE+" "+LIBRARIES
# Print out the compile string
print("============v (Command running on terminal) v===========================")
print("Compilng on: "+platform.system())
print(compileString)
print("========================================================================")

exit_code = os.system(compileString)
exit(0 if exit_code==0 else 1)
# ========================= Building the Executable ========================== #
