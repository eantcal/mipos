@echo off
rem Launch a mipOS ARM firmware under QEMU from Windows cmd.exe.
rem
rem Usage:
rem   scripts\run-qemu.cmd                 filesystem demo
rem   scripts\run-qemu.cmd console         interactive mipOS console (CLI)
rem   scripts\run-qemu.cmd helloworld      tick demo
rem   scripts\run-qemu.cmd console -Gdb    start halted with GDB server :1234
rem
rem All arguments are forwarded to scripts\run-qemu.ps1.

setlocal
set "SCRIPT_DIR=%~dp0"
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%run-qemu.ps1" %*
exit /b %ERRORLEVEL%
