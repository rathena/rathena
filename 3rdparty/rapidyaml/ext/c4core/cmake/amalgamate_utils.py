import re
import os


class cmtfile:
    """commented file"""
    def __init__(self, filename):
        self.filename = filename
    def __str__(self):
        return self.filename


class cmttext:
    """commented text"""
    def __init__(self, text):
        self.text = text
    def __str__(self):
        return self.text


class ignfile:
    """ignore file"""
    def __init__(self, filename):
        self.filename = filename
    def __str__(self):
        return self.filename


class hdrfile:
    """header file, with custom include guard"""
    def __init__(self, filename, incpattern, include_guard=None):
        self.filename = filename
        self.incpattern = incpattern
        self.include_guard = include_guard
    def __str__(self):
        return self.filename


class injfile:
    """header file, to be injected at the first include point"""
    def __init__(self, filename, incpattern):
        self.filename = filename
        self.incpattern = incpattern
    def __str__(self):
        return self.filename


class injcode:
    """direct code to inject"""
    def __init__(self, code):
        self.code = code
    def __str__(self):
        return self.code


class onlyif:
    def __init__(self, condition, obj):
        self.condition = condition
        self.obj = obj


def catfiles(filenames, rootdir,
             include_regexes,
             definition_macro,
             repo,
             result_incguard):
    sepb = "//" + ("**" * 40)
    sepf = "//" + ("--" * 40)
    to_inject = {}
    custom_include_guards = {}
    def banner(s):
        return f"\n\n\n{sepb}\n{sepf}\n// {s}\n// {repo}/{s}\n{sepf}\n{sepb}\n\n"
    def footer(s):
        return f"\n\n// (end {repo}/{s})\n"
    def incguard(filename):
        return custom_include_guards.get(filename,
                                         f"{filename.replace('.','_').replace('/','_').upper()}_")
    def replace_include(rx, match, line, guard):
        line = line.rstrip()
        incl = match.group(1)
        if to_inject.get(incl) is None:
            if guard is None:
                guard = incguard(incl)
            return f"""// amalgamate: removed include of
// {repo}/src/{incl}
//{line}
#if !defined({guard}) && !defined(_{guard})
#error "amalgamate: file {incl} must have been included at this point"
#endif /* {guard} */\n
"""
        else:
            entry = to_inject[incl]
            del to_inject[incl]
            return append_file(entry.filename)
    def append_file(filename, guard=None):
        s = ""
        with open(filename) as f:
            for line in f.readlines():
                for rx in include_regexes:
                    match = rx.match(line)
                    if match:
                        line = replace_include(rx, match, line, guard)
                s += line
        return s
    def append_cpp(filename):
        return f"""#ifdef {definition_macro}
{append_file(filename)}
#endif /* {definition_macro} */
"""
    def is_src(filename):
        return filename.endswith(".cpp") or filename.endswith(".c")
    def cmtline(line, more=""):
        if len(line.strip()) > 0:
            return f"// {line}{more}"
        else:
            return "//\n"
    out = ""
    for entry in filenames:
        if isinstance(entry, onlyif):
            if entry.condition:
                entry = entry.obj
            else:
                continue
        if isinstance(entry, ignfile):
            pass
        elif isinstance(entry, cmttext):
            for line in entry.text.split("\n"):
                out += cmtline(line, "\n")
        elif isinstance(entry, cmtfile):
            filename = f"{rootdir}/{entry.filename}"
            out += banner(entry.filename)
            with open(filename) as file:
                for line in file.readlines():
                    out += cmtline(line)
        elif isinstance(entry, injcode):
            out += f"\n{entry.code}\n"
        elif isinstance(entry, injfile):
            entry.filename = f"{rootdir}/{entry.filename}"
            to_inject[entry.incpattern] = entry
        else:
            filename = f"{rootdir}/{entry}"
            out += banner(entry)
            if isinstance(entry, hdrfile):
                if entry.include_guard is not None:
                    custom_include_guards[entry.incpattern] = entry.include_guard
                out += append_file(filename, entry.include_guard)
            else:
                assert isinstance(entry, str)
                if is_src(filename):
                    out += append_cpp(filename)
                else:
                    out += append_file(filename)
            out += footer(entry)
    return f"""#ifndef {result_incguard}
{out}
#endif /* {result_incguard} */
"""

def include_only_first(file_contents: str):
    rx = [
        re.compile(r'^\s*#\s*include "(.*?)".*'),
        re.compile(r'^\s*#\s*include <(.*?)>.*'),
    ]
    already_included = {}
    out = ""
    for line in file_contents.split("\n"):
        for expr in rx:
            match = expr.match(line)
            if match:
                incl = match.group(1)
                if already_included.get(incl) is None:
                    already_included[incl] = line
                    if incl.endswith(".h"):
                        cpp_version = f"c{incl[:-2]}"
                        already_included[cpp_version] = line
                    elif incl.startswith("c") and not (incl.endswith(".h") or incl.endswith(".hpp")):
                        c_version = f"{incl[1:]}.h"
                        already_included[c_version] = line
                else:
                    line = f"//included above:\n//{line}"
                break
        out += line
        out += "\n"
    return out


def mkparser(**bool_args):
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("output", default=None, nargs='?', help="output file. defaults to stdout")
    for k, (default, help) in bool_args.items():
        # https://stackoverflow.com/questions/15008758/parsing-boolean-values-with-argparse
        feature = parser.add_mutually_exclusive_group(required=False)
        yes = '--' + k
        no = '--no-' + k
        if default:
            yes_default = "this is the default"
            no_default = f"the default is {yes}"
        else:
            yes_default = f"the default is {no}"
            no_default = "this is the default"
        feature.add_argument(yes, dest=k, action='store_true', help=f"{help}. {yes_default}.")
        feature.add_argument(no, dest=k, action='store_false', help=f"{help}. {no_default}.")
        parser.set_defaults(**{k: default})
    return parser


def file_put_contents(filename: str, contents: str):
    if filename is None:
        print(contents)
    else:
        dirname = os.path.dirname(filename)
        if dirname:
            os.makedirs(dirname, exist_ok=True)
        with open(filename, "w") as output:
            output.write(contents)
