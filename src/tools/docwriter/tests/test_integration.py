#
#  test_integration.py
#
#    Integration test for docwriter.
#
#  Copyright 2018 by
#  Nikhil Ramakrishnan.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

"""Docwriter Integration tests.

This is a simple integration test that builds Docwriter
documentation against a test file.

From the root of the Docwriter git repo, use:
    python -m pytest tests/test_integration.py
"""

import logging
import subprocess

log = logging.getLogger('docwriter')

def test_integration( capfd ):

    log.propagate = False
    stream = logging.StreamHandler()
    formatter = logging.Formatter(
        "\033[1m\033[1;32m *** %(message)s *** \033[0m")
    stream.setFormatter(formatter)
    log.addHandler(stream)
    log.setLevel(logging.DEBUG)

    base_cmd = ['python', 'docwriter.py', '--prefix=test',
                '--title=Docwriter Test', '--output=./tests/output',
                '--verbose' ]
    folders  = ['./tests/assets/*.c']

    log.debug("Building markdown docs.")
    command = base_cmd + folders
    # run the command
    subprocess.check_call( command )
    # capture output to check for warnings
    captured = capfd.readouterr()
    # print the logs on failure
    print( captured.err )
    # fail if there are warnings
    assert not "WARNING" in captured.err

# eof
