rem Don't do this at home!

def fn a (x)= fn b (x) + fn b (x)
def fn b (x)= fn a (x) * fn a (x)
print fn b (2)
