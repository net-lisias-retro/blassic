def fn fact (n)
	if n = 1 then fact= 1 else fact= n * fn fact (n - 1)
fn end

for i= 1 to 10
	print i, fn fact (i)
next

end