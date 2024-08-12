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
    sys.stderr.write("factory.py: ")
    sys.stderr.write(msg)
    sys.stderr.write("\n")


os.makedirs(".pio/data", exist_ok=True)

for filename in ["config.html", "logo-icon.png", "logo.png"]:
    skip = False

    if os.path.isfile(".pio/data/" + filename + ".timestamp"):
        with open(
            ".pio/data/" + filename + ".timestamp", "r", -1, "utf-8"
        ) as timestampFile:
            if os.path.getmtime("data/" + filename) == float(timestampFile.readline()):
                skip = True

    if skip:
        status("[data.py] " + filename + " up to date")
        continue

    with open("data/" + filename, "rb") as inputFile:
        with gzip.open(".pio/data/" + filename + ".gz", "wb") as outputFile:
            status(
                "[data.py] gzip 'data/"
                + filename
                + "' to '.pio/data/"
                + filename
                + ".gz'"
            )
            outputFile.writelines(inputFile)
    with open(
        ".pio/data/" + filename + ".timestamp", "w", -1, "utf-8"
    ) as timestampFile:
        timestampFile.write(str(os.path.getmtime("data/" + filename)))
