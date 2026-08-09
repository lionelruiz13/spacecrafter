# F30 / INTENT §5.60 - identify the single 2,684,360,960-byte device allocation.
#
# The row records the size and the device limit it exceeds; it does NOT record
# WHICH allocation it is, nor whether it is one buffer or one pool. This script
# answers exactly that, at the callsite.
#
# Channel: a breakpoint at the ENTRY of the loader's vkAllocateMemory (`break *`,
# so no prologue is skipped and the SysV argument registers are intact), reading
# allocationSize out of the VkMemoryAllocateInfo the caller passed
# (offset 16: sType 4 + pad 4 + pNext 8). EVERY allocation is printed, so the
# instrument's own liveness is visible in the record (positive control), and the
# ones above 1 GiB additionally dump the C++ backtrace - which is where the
# requesting subsystem is named, our frames carrying RelWithDebInfo symbols.
set pagination off
set confirm off
set breakpoint pending on
set print elements 0
# The app installs its own SIGUSR1 stall watchdog; do not let gdb eat signals.
handle SIGUSR1 nostop noprint pass

break *vkAllocateMemory
commands
silent
set $sz = *(unsigned long *)($rsi + 16)
set $mti = *(unsigned int *)($rsi + 24)
printf "ALLOC size=%llu memoryTypeIndex=%u\n", $sz, $mti
if $sz > 1073741824
printf "=== BIG ALLOCATION %llu bytes ===\n", $sz
bt 25
printf "=== END BIG ALLOCATION ===\n"
end
continue
end

run
printf "=== PROGRAM ENDED ===\n"
bt 25
quit
