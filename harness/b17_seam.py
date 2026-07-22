#!/usr/bin/env python3
# B17 - view_offset / zoom_offset command-spelling probe (INTENT 13.B B17).
#
# Grounds the B17 finding empirically on the running (default = NEW path)
# process. It asserts NOTHING about the new render (there is nothing to assert:
# the offset never reaches the new path - that IS the seam). It confirms:
#   (1) the runtime command `set zoom_offset=X` reaches Core::setViewOffset
#       (a PROBE hit with offset==X in b17_probe.gdb) - the command SPELLING is
#       real on the running process, not fiction (the 8x command-spelling trap);
#   (2) a BOGUS spelling (`set zoom_ofset=X`) produces NO hit - bogus => 0 fire.
# The config channel is confirmed separately: b17_probe.gdb hits
# Core::setViewOffset at startup with offset==0 (the config `view_offset=0`).
#
# New-path effect is source-provable, not measured here: there is NO
# Camera::setViewOffset / no new-path reader of the offset (grep-clean); the
# sink Core::setViewOffset touches only the OLD navigator (core.cpp:2111-2124).
#
# PRECONDITION: app fresh-launched UNDER b17_probe.gdb, enable_tcp, default path.

import socket, time, sys

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 7805))
time.sleep(0.5)


def send(cmd, pause=1.0):
    sock.sendall((cmd + "\n").encode())
    print("SENT:", cmd)
    time.sleep(pause)
    try:
        sock.settimeout(0.3); sock.recv(4096)
    except socket.timeout:
        pass
    sock.settimeout(None)


send("timerate rate 0")
# COMMAND SYNTAX: `set NAME VALUE`, SPACE-separated - parseCommand reads
# `command` then (key value) pairs [app_command_interface.cpp:144-166]; an `=`
# form yields ZERO args (the 8x command-spelling trap - verified live here).
# (1) valid spellings, distinct values so each hit is individually attributable.
send("set zoom_offset 0.2")
send("set zoom_offset 0.35")
# (2) bogus NAME, correct syntax - must NOT hit (bogus name => 0 fire).
send("set zoom_ofset 0.4")
send("set zoomoffset 0.4")
# reset
send("set zoom_offset 0")
print("done")
sys.exit(0)
