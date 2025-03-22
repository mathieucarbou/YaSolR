import gzip
import os
import sys

quiet = False


def status(msg):
    """Print status message to stderr"""
    if not quiet:
        critical(msg)


def critical(msg):
    """Print critical message to stderr"""
    sys.stderr.write("embed.py: ")
    sys.stderr.write(msg)
    sys.stderr.write("\n")


os.makedirs(".pio/embed", exist_ok=True)

for filename in ["config.html", "logo-icon.png", "logo.png"]:
    skip = False

    if os.path.isfile(".pio/embed/" + filename + ".timestamp"):
        with open(
            ".pio/embed/" + filename + ".timestamp", "r", -1, "utf-8"
        ) as timestampFile:
            if os.path.getmtime("embed/" + filename) == float(timestampFile.readline()):
                skip = True

    if skip:
        status("[embed.py] " + filename + " up to date")
        continue

    with open("embed/" + filename, "rb") as inputFile:
        with gzip.open(".pio/embed/" + filename + ".gz", "wb") as outputFile:
            status(
                "[embed.py] gzip 'embed/"
                + filename
                + "' to '.pio/embed/"
                + filename
                + ".gz'"
            )
            outputFile.writelines(inputFile)
    with open(
        ".pio/embed/" + filename + ".timestamp", "w", -1, "utf-8"
    ) as timestampFile:
        timestampFile.write(str(os.path.getmtime("embed/" + filename)))
