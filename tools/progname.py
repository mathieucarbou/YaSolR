# SPDX-License-Identifier: MIT
#
# Copyright (C) 2023-2026 Mathieu Carbou
#
Import("env")

import os
import sys


def get_lang(e):
    for define in e.get("CPPDEFINES", []):
        if (
            isinstance(define, (list, tuple))
            and len(define) >= 2
            and str(define[0]) == "YASOLR_LANG"
        ):
            return "fr" if "FR" in str(define[1]).upper() else "en"
    return "en"


pioenv = env["PIOENV"]
lang = get_lang(env)
ref = os.environ.get("BUILD_REF", "unknown")
progname = "YaSolR-%s-%s-%s.OTA" % (ref, pioenv, lang)

env.Replace(PROGNAME=progname)
sys.stderr.write("progname.py: PROGNAME -> %s\n" % progname)
