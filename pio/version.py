import subprocess
import os
from datetime import datetime, timezone


def get_build_flag():
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
    is_tag = branch.startswith("v") and branch.len() == 6

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

    return (
        '-D APP_VERSION=\\"'
        + version
        + '\\" '
        + '-D BUILD_BRANCH=\\"'
        + branch
        + '\\" '
        + '-D BUILD_HASH=\\"'
        + short_hash
        + '\\" '
        + '-D BUILD_TIMESAMP=\\"'
        + datetime.now(timezone.utc).isoformat()
        + '\\"'
    )


build_flags = get_build_flag()

if "SCons.Script" == __name__:
    print("Firmware Revision: " + build_flags)
    Import("env")
    env.Append(BUILD_FLAGS=[get_build_flag()])
elif "__main__" == __name__:
    print(build_flags)
