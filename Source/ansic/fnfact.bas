def fn fact (n)
	local i
	fact= 1
	for i= 1 to n
		fact= fact * i
	next
fn end

for i= 1 to 10
	print i, fn fact (i), i
next

end
