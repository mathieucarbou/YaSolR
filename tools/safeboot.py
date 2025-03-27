Import("env")
import sys
import urllib.request

quiet = False


def status(msg):
    """Print status message to stderr"""
    if not quiet:
        critical(msg)


def critical(msg):
    """Print critical message to stderr"""
    sys.stderr.write("safeboot.py: ")
    sys.stderr.write(msg)
    sys.stderr.write("\n")


# open "/safeboot" on target to restart in SafeBoot-mode
def safeboot(source, target, env):
    upload_protocol = env.GetProjectOption("upload_protocol")
    upload_port = env.GetProjectOption("upload_port")
    if upload_protocol != "espota":
        critical("Wrong upload protocol (%s)" % upload_protocol)
        raise Exception("Wrong upload protocol!")
    else:
        status("Trying to activate SafeBoot on: %s" % upload_port)
        safeboot_path = env.GetProjectOption("custom_safeboot_restart_path", "/safeboot")
        req = urllib.request.Request("http://" + upload_port + safeboot_path, method="POST")
        try:
            urllib.request.urlopen(req)
        except urllib.error.URLError as e:
            critical(e)
            # Raise exception when SafeBoot cannot be activated
            pass

        status("Activated SafeBoot on: %s" % upload_port)


env.AddPreAction("upload", safeboot)
env.AddPreAction("uploadfs", safeboot)
env.AddPreAction("uploadfsota", safeboot)
