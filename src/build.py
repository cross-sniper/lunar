import subprocess
import argparse
import os
def read_file(filename):
    if not os.path.exists(filename):
        return ""
    else:
        with open(filename, "r") as f:
            return f.read()

def is_older(filename1, filename2):
    if not os.path.exists(filename1) or not os.path.exists(filename2):
        return True
    return os.path.getmtime(filename1) < os.path.getmtime(filename2)

# Compiler and flags
CC = "g++"
DEPENDENCIES = read_file("dependencies.txt").replace('\n',' ')
CFLAGS = DEPENDENCIES + " -Wall -O2 -fno-stack-protector -fno-common -march=native"
LUNAR_INCLUDES = "-lcurl -lraylib"
LIBS = "-lm -ldl -lreadline -lcurl"

# Source files
CORE_O = ["lapi.o", "lcode.o", "lctype.o", "ldebug.o", "ldo.o", "ldump.o",
          "lfunc.o", "lgc.o", "llex.o", "lmem.o", "lobject.o", "lopcodes.o",
          "lparser.o", "lstate.o", "lstring.o", "ltable.o", "ltm.o", "lundump.o",
          "lvm.o", "lzio.o", "ltests.o"]

AUX_O = ["lauxlib.o"]

LIB_O = ["lbaselib.o", "ldblib.o", "liolib.o", "lmathlib.o", "loslib.o",
         "lcorolib.o", "lstrlib.o", "ltablib.o", "lutf8lib.o", "loadlib.o", "linit.o"]

LUA_O = ["lua.o"]

# Output files
CORE_T = "liblua.a"
LUA_T = "lunar"
ALL_T = [CORE_T]
ALL_O = CORE_O + AUX_O + LIB_O + LUA_O
ALL_A = [CORE_T]

def compile_source(source_file, object_file):
    if is_older(source_file, object_file):
        compile_command = f"{CC} {CFLAGS} -c {source_file} -o {object_file}"
        print("compiling " + source_file)
        subprocess.run(compile_command, shell=True, check=True)
    else:
        print("skipping " + source_file)

def link_lua():
    link_command = f"{CC} -o {LUA_T} {LUA_O[0]} {CORE_T} {LIBS} {LUNAR_INCLUDES}"
    print("linking " + LUA_T)
    subprocess.run(link_command, shell=True, check=True)

def archive_lua():
    archive_command = f"ar rc {CORE_T} {' '.join(ALL_O)}"
    print("archiving " + CORE_T)
    subprocess.run(archive_command, shell=True, check=True)

def ranlib_lua():
    ranlib_command = f"ranlib {CORE_T}"
    print("ranlib " + CORE_T)
    subprocess.run(ranlib_command, shell=True, check=True)

def clean():
    clean_command = f"rm -f {' '.join(ALL_T)} {' '.join(ALL_O)}"
    print("cleaning")
    subprocess.run(clean_command, shell=True, check=True)
#add a optional flag, to clean the build

args = argparse.ArgumentParser()
args.add_argument('--clean', action='store_true', help='Clean the build', default=False)
args = args.parse_args()

if __name__ == "__main__":
    try:
        for source_file, object_file in zip(CORE_O + AUX_O + LIB_O + LUA_O, ALL_O):
            compile_source(source_file.replace(".o", ".c"), object_file)

        archive_lua()
        ranlib_lua()
        link_lua()
        print(f"compiled and linked {LUA_T} with flags {CFLAGS}")

    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
    finally:
        if args.clean:
            clean()
