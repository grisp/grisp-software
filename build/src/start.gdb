# reload the binary
def reset
	print "Resetting target"
	monitor reset_and_run_bootloader
	load
end

# enable history
set history filename ~/.gdb_history
set history save

# some behavioural settings
set print pretty on
set pagination off
set confirm off

# connect
target extended-remote localhost:3333
monitor gdb_breakpoint_override hard

# load application for the first time
reset
