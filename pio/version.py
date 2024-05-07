import subprocess
import os
import re
from datetime import datetime, timezone

Import("env")


def do_main():
    # hash
    ret = subprocess.run(
        ["git", "rev-parse", "HEAD"], stdout=subprocess.PIPE, text=True, check=False
    )  # Uses any tags
    full_hash = ret.stdout.strip()
    short_hash = full_hash[:7]

    # branch
    ret = subprocess.run(
        ["git", "symbolic-ref", "--short", "HEAD"],
        stdout=subprocess.PIPE,
        text=True,
        check=False,
    )  # retrieve branch name
    branch = ret.stdout.strip()
    ref_name = os.environ.get("GITHUB_REF_NAME")
    if ref_name:
        # on GitHub CI
        branch = ref_name
    branch = branch.replace("/", "")
    branch = branch.replace("-", "")
    branch = branch.replace("_", "")

    # is_tag ?
    is_tag = branch.startswith("v") and len(branch) == 6

    # local modifications ?
    has_local_modifications = False
    if not ref_name:
        # Check if the source has been modified since the last commit
        ret = subprocess.run(
            ["git", "diff-index", "--quiet", "HEAD", "--"],
            stdout=subprocess.PIPE,
            text=True,
            check=False,
        )
        has_local_modifications = ret.returncode != 0

    version = branch
    if not is_tag:
        version += "_" + short_hash
    if has_local_modifications:
        version += "_modified"

    # version = "v2.40.2_rc1"
    tagPattern = re.compile("^v[0-9]+.[0-9]+.[0-9]+([_-][a-zA-Z0-9]+)?$")
    constantFile = os.path.join(env.subst("$BUILD_DIR"), "__compiled_constants.c")
    with open(constantFile, "w") as f:
        f.write(
            f'const char* __COMPILED_APP_VERSION__ = "{version[1:] if tagPattern.match(version)  else version}";\n'
            f'const char* __COMPILED_BUILD_BRANCH__ = "{branch}";\n'
            f'const char* __COMPILED_BUILD_HASH__ = "{short_hash}";\n'
            f'const char* __COMPILED_BUILD_NAME__ = "{env["PIOENV"]}";\n'
            f'const char* __COMPILED_BUILD_TIMESTAMP__ = "{datetime.now(timezone.utc).isoformat()}";\n'
        )

    env.AppendUnique(PIOBUILDFILES=[constantFile])

do_main()
