import os

BASE_COMMAND = "vagrant ssh -c '%s'"

def vm_exec(cmd, verbose=False):
    """
    Executes a bash command within the virtual machine. If 'verbose' 
    is set to True, then this function will print the `vagrant ssh` 
    call which causes the command to be executed.
    """
    if verbose:
        print "[Exec] " + BASE_COMMAND % cmd
    os.system(BASE_COMMAND % cmd)

def vm_run_prog(program, args, verbose=False):
    """
    Executes a program, with arguments, in the virtual machine.
    If 'verbose' is set to True, then this function will also
    print the `vagrant ssh` command used to execute the program.
    """
    vm_exec("%s %s" % (program, " ".join(args)), verbose=verbose)