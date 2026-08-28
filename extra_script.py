Import("env")
import os
import subprocess

# Windows workaround: on this machine, PlatformIO/SCons's default process-spawning path
# (cmd.exe based) intermittently corrupts or misroutes build tool invocations. SCons
# sometimes hands SPAWN an args list that was produced by naively whitespace-splitting a
# pre-quoted command string, so a single quoted "C:\path with space\thing.exe" argument
# (and, worse, a quote that opens mid-token like -Wl,-Map="C:\path with space\out.map")
# arrives as multiple broken tokens unless something re-parses the whole line with quotes
# in mind. shlex.split(..., posix=False) was tried here but only honors a quote that
# starts a fresh word, not one appearing mid-token as in the -Wl,-Map= case above - so
# this uses a small hand-rolled tokenizer that toggles quote state anywhere in the
# string, splitting on whitespace only outside quotes and dropping the quote chars.
# Reconstructs the true argv from the joined command line, then dispatches directly
# (shell=False) with the project dir pinned as cwd - falling back to cmd.exe only for
# genuine shell builtins (e.g. `del`) that have no real .exe backing.
PROJECT_DIR = env.subst("$PROJECT_DIR")


def _tokenize(cmdline):
    # Some framework build steps (e.g. ESP8266's ARDUINO_BOARD_ID define) embed a
    # backslash-escaped quote (\") to put a literal " into the token, e.g.
    # -DARDUINO_BOARD_ID=\"esp12e\" must become the token -DARDUINO_BOARD_ID="esp12e"
    # (literal quotes kept, since that's what makes the macro a C string). A bare
    # '"' still just toggles quoting and is dropped, matching the rest of the line.
    tokens = []
    current = []
    in_quotes = False
    i, n = 0, len(cmdline)
    while i < n:
        c = cmdline[i]
        if c == "\\" and i + 1 < n and cmdline[i + 1] == '"':
            current.append('"')
            i += 2
            continue
        if c == '"':
            in_quotes = not in_quotes
        elif c.isspace() and not in_quotes:
            if current:
                tokens.append("".join(current))
                current = []
        else:
            current.append(c)
        i += 1
    if current:
        tokens.append("".join(current))
    return tokens


def _spawn(sh, escape, cmd, args, spawnenv):
    full_env = dict(os.environ)
    full_env.update(spawnenv)
    cmdline = " ".join(str(a) for a in args)
    real_args = _tokenize(cmdline)
    try:
        return subprocess.run(real_args, env=full_env, shell=False, cwd=PROJECT_DIR).returncode
    except OSError:
        return subprocess.run(cmdline, env=full_env, shell=True, cwd=PROJECT_DIR).returncode

env["SPAWN"] = _spawn
