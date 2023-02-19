     10 REM
     20 REM Dynamic Link sample.
     30 REM
     40 win= OSFAMILY$ = "windows"
     50 REM Change the library to match your version.
     60 IF win THEN lib$= "kernel32" : proc$= "GetCurrentProcessId" : ELSE lib$= "/lib/libc.so.6" : proc$= "getpid"
     70 pid= USR (lib$, proc$)
     80 PRINT "The PID of this process is: "; pid; " hex "; HEX$ (pid)
     90 IF NOT win THEN END
    100 tid= USR("kernel32", "GetCurrentThreadId")
    110 PRINT "The thread ID is: "; tid; " hex "; HEX$ (tid)
    120 END
