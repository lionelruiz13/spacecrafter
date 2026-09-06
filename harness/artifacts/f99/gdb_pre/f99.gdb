set pagination off
set confirm off
handle SIGUSR1 nostop noprint pass
handle SIGPIPE nostop noprint pass
run
echo \n===== SIGNAL CAUGHT =====\n
info program
echo \n===== BACKTRACE (10 innermost) =====\n
bt 10
echo \n===== FRAME 0 =====\n
frame 0
info frame
echo \n===== THE PARENT POINTER AT THE FAULT =====\n
info locals
echo \n===== SOURCE AT THE FAULT =====\n
list
kill
quit
