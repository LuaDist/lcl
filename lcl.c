/*
* lcl.c
* command line evaluator for Lua 5.1
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 07 Jun 2010 09:54:48
* This code is hereby placed in the public domain.
*/

#include "lua.h"
#include "lauxlib.h"

static int nexttoken(lua_State *L, const char **S)
{
	const char *s=*S;
	const char *b;
	size_t l;
	int t;
	while (*s==' ' || *s=='\t') s++;
	if (*s==0 || *s=='\n' || *s==';' || *s=='(' || *s==')') {
		*S=s+1;
		return *s;
	}
	if (*s=='"' || *s=='\'') {
		int q=*s++;
		t='Q';
		b=s;
		for (l=0; *s!=q && *s!='\n' && *s!=0; s++,l++);
		if (*s==q) s++; else
		{
			lua_settop(L,3);
			lua_pushboolean(L,0);
			lua_pushliteral(L,"syntax");
			lua_pushliteral(L,"unfinished string");
			return -1;
		}
	}
	else {
		t='W';
		b=s;
		for (l=0; *s!=' '  && *s!='\t' && *s!='\n' &&
			  *s!='"'  && *s!='\'' &&
			  *s!='('  && *s!=')'  && *s!=0; s++,l++);
		if (*b=='$') {
			t='$';
			b++; l--;
		}
	}
	luaL_checkstack(L,2,"too many arguments");
	lua_pushlstring(L,b,l);
	if (t=='W' && lua_isnumber(L,-1))
	{
		lua_pushnumber(L,lua_tonumber(L,-1));
		lua_remove(L,-2);
		t='N';
	}
	*S=s;
	return t;
}

static int docall(lua_State *L, int env, int n, int r)
{
	if (n==0) return 1;
	lua_pushvalue(L,-n);
	lua_pushvalue(L,-1);
	lua_replace(L,3);
	if (lua_type(L,-1)==LUA_TSTRING) lua_gettable(L,env);			
	lua_replace(L,-n-1);
	if (lua_pcall(L,n-1,r,0)==0) {
		return 1;
	}
	else {
		lua_settop(L,3);
		lua_pushboolean(L,0);
		lua_pushvalue(L,3);
		lua_pushvalue(L,-3);
		return 0;
	}
}

static int Lunpack(lua_State *L)		/** unpack(s) */
{
	const char *s=luaL_checkstring(L,1);
	lua_settop(L,1);
	for (;;) {
		int t=nexttoken(L,&s);
		if (t<0) return 3;
		if (t==0) break;
	}
	return lua_gettop(L)-1;
}

static int Leval(lua_State *L)			/** eval(s) */
{
	const char *s=luaL_checkstring(L,1);
	int env= lua_isnoneornil(L,2) ? LUA_GLOBALSINDEX : 2;
	lua_settop(L,3);
	for (;;) {
		int t=nexttoken(L,&s);
		if (t<0) return 3;
		else if (t=='$')
			lua_gettable(L,env);
		else if (t=='\n' || t==';' || t==0)
		{
			if (!docall(L,env,lua_gettop(L)-3,0)) return 3;
			if (t==0) break;
		}
	}
	lua_pushboolean(L,1);
	return 1;
}

static int dosexp(lua_State *L, const char **s, int env)
{
	for (;;) {
		int t=nexttoken(L,s);
		if (t<=0) return t;
		else if (t=='W')
			lua_gettable(L,env);
		else if (t=='(') {
			int top=lua_gettop(L);
			if (dosexp(L,s,env)!=')') {
				lua_settop(L,3);
				lua_pushboolean(L,0);
				lua_pushliteral(L,"syntax");
				lua_pushliteral(L,"unfinished list");
				return -1;
			}
			if (!docall(L,env,lua_gettop(L)-top,LUA_MULTRET)) return -1;
		}
		else if (t==')')
			return t;
	}
	return 0;
}

static int Lsexp(lua_State *L)		/** sexp(s) */
{
	const char *s=luaL_checkstring(L,1);
	int env= lua_isnoneornil(L,2) ? LUA_GLOBALSINDEX : 2;
	lua_settop(L,3);
	if (dosexp(L,&s,env)<0) return 3;
	return lua_gettop(L)-3;
}

static const luaL_Reg R[] =
{
	{ "eval",	Leval},
	{ "sexp",	Lsexp},
	{ "unpack",	Lunpack},
	{ NULL,		NULL	}
};

LUALIB_API int luaopen_cl(lua_State *L)
{
	luaL_register(L,"cl",R);
	return 1;
}
