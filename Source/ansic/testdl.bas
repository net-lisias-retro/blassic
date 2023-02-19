rem testdl.bas

if osfamily$ = "windows" then lib$= "testdl" else lib$= "./testdl.so"

a$= "Esto es una prueba"
gosub call_it
a$= "This is a test"
gosub call_it
end

label call_it
print a$
n= usr (lib$, "testfunc", varptr (a$) )
if n <> 0 then print "Error calling dynamic linked function." else print a$

return
