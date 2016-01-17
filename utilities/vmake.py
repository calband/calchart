"""
This python script will run the `make` command inside of the virtual
machine (with arguments). The virtual machine must be running
before this script is used.

Example usage: The following command will cause `make generate` to 
be run within the virtual machine.
    python vmake.py generate
"""

import sys

from calchart_pyutils.vm.vm_exec import vm_run_prog

if __name__ == '__main__':
    vm_run_prog("make", sys.argv[1:])
