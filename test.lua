require"cl"

function set(name,value)
	_G[name]=value
end

function get(name)
	return name,_G[name]
end

function list(...)
	return ...
end

function test(w,s)
	print("-------------------------------------------------------",w)
	print("input")
	print(s)
	print("output")
	print("return\n",cl[w](s))
end

set("true",true)
set("false",false)
set("nil",nil) -- silly
set("+", function (x,y) return x+y end)

test("eval",[[
	set me	tarzan
	set you	jane
	set us	"tarzan & jane"
	print us $me and $you
]])

test("sexp",[[
	(set 'me' "tarzan")
	(set 'you' "jane")
	(set 'us' "tarzan & jane")
	(print 'us' me "and" you)
	(+ 12 34) 2010
	"and" (list (get 'me') (get 'you'))
]])
