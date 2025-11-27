Import("env")
import logging
import subprocess

from datetime import date

today = date.today()

try:
    revision = (
        subprocess.check_output(
            [
                "git",
                "describe",
                "--abbrev=8",
                "--dirty",
                "--always",
                "--tags",
            ]
        )
        .strip()
        .decode("utf-8")
    )

    # Have to do some magic to escape the cpp literal
    revision_escaped = revision.replace("\\", "\\\\").replace('"', '\\"')
    env.Append(CPPDEFINES=[("REVISION", f'\\"{revision_escaped}\\"')])
except Exception as e:
    logging.warning("Getting git revision failed!! Check that you have git installed?")
    logging.warning(e)

    env.Append(CPPDEFINES=[("REVISION", '\\"Unknown\\"')])

try:
    host = (
        subprocess.check_output(
            [
                "hostname",
            ]
        )
        .strip()
        .decode("utf-8")
    )

    # Escape the host string properly for C++ string literal
    host_escaped = host.replace("\\", "\\\\").replace('"', '\\"')
    env.Append(CPPDEFINES=[("FWHOST", f'\\"{host_escaped}\\"')])
except Exception as e:
    logging.warning("Getting host failed!! Are you using a UNIX compatible system?")
    logging.warning(e)

    env.Append(CPPDEFINES=[("FWHOST", '\\"Unknown\\"')])

try:
    build_date = (
        subprocess.check_output(
            [
                "date",
                "+%s",
            ]
        )
        .strip()
        .decode("utf-8")
    )

    # Escape the build_date string properly for C++ string literal
    build_date_escaped = build_date.replace("\\", "\\\\").replace('"', '\\"')
    env.Append(CPPDEFINES=[("BUILD_DATE", f'\\"{build_date_escaped}\\"')])
except Exception as e:
    logging.warning(
        "Getting build date failed!! Are you using a UNIX compatible system?"
    )
    logging.warning(e)

    env.Append(CPPDEFINES=[("BUILD_DATE", '\\"Unknown\\"')])

try:
    username = (
        subprocess.check_output(
            [
                "id",
                "-u",
                "-n",
            ]
        )
        .strip()
        .decode("utf-8")
    )

    # Cleanup CI
    if username == "root":
        username = "git"
        host = "git"

    # Escape the username string properly for C++ string literal
    username_escaped = username.replace("\\", "\\\\").replace('"', '\\"')
    env.Append(CPPDEFINES=[("USERNAME", f'\\"{username_escaped}\\"')])
except Exception as e:
    logging.warning("Getting username failed!! Are you using a UNIX compatible system?")
    logging.warning(e)

    env.Append(CPPDEFINES=[("USERNAME", '\\"Unknown\\"')])

# Uncomment to enable debugging
# env.Append(CPPDEFINES=[("DEBUG", "1")])
