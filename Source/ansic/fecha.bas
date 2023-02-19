rem fecha.bas
rem Escribe la fecha en formato español abreviado.

gosub fecha
print fecha$
end

label fecha

if fecha_init = 0 then gosub fecha_init

local f$, m, mes$

f$= date$
m= val (left$ (f$, 2) )
rem mes$= mes_abrev$ (m)
mes$= mid$ ("EneFebMarAbrMayJunJulAgoSepOctNovDic", m * 3 - 2, 3)
fecha$= mid$ (f$, 4, 2) + "-" + mes$ + "-" + right$ (f$, 4)

return

label fecha_init

dim mes_abrev$ (12)
restore fecha_init_data
for i= 1 to 12
	read mes_abrev$ (i)
next

fecha_init= 1

return

label fecha_init_data
data Ene, Feb, Mar, Abr, May, Jun, Jul, Ago, Sep, Oct, Nov, Dic

rem Fin de fecha.bas
