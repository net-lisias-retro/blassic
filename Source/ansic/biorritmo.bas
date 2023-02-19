5 rem Original para Spectrum, El Ordenador Personal N. 29, 1984
10 data 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
20 dim m (12) : rem J. Antonio...
30 for f= 1 to 12: read a: let m (f)= a: next f
100 input "Nacimiento año: "; a1
102 input "Nacimiento mes: "; m1
104 input "Nacimiento dia: "; d1
110 input "Biorritmo año: "; a2
112 input "Biorritmo mes: "; m2
120 mode "spectrum"
200 let a3= a2 - a1
210 let b= int (a3 / 4)
220 let dd= m (m1) + d1
230 let dt= 365 * a3 + b + m (m2) - dd
240 let f= dt / 23 - int (dt / 23): let e= dt / 28 - int (dt / 28)
250 let i= dt / 33 - int (dt / 33)
260 print : print "  Mes: "; m2; "   Dias:"; dt
270 for n= 0 to 31: print at 11, n; "|"; at 0, n; "#"; at 20, n; "#";
280 if n < 21 then print at n, 0; "#"; at n, 31; "#";
290 next n
300 for x= 1 to 255
310 let y1= 24 * sin ( (x + 184 * f) * pi / 92): let y2= 40 * sin ( (x + 224 * e) * pi / 112): let y3= 70 * sin ( (x+264 * y) * pi / 132)
320 plot x, y1 + 87: plot x, y2 + 87: plot x, y3 + 87
330 next x
390 print at 19, 0; "# Fisico  Emocional  Intelec."
10000 get a$
