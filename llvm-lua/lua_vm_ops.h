
#ifndef lua_vm_ops_h
#define lua_vm_ops_h

#ifdef __cplusplus
extern "C" {
#endif

#include "lua_core.h"
#include "lobject.h"

typedef struct {
  TValue *k;
  LClosure *cl;
} func_state;

typedef enum {
	HINT_T_NONE = 0,
	HINT_T_C_NUM_CONSTANT,
} hint_t;

typedef enum {
	VAR_T_VOID = 0,
	VAR_T_INT,
	VAR_T_ARG_A,
	VAR_T_ARG_B,
	VAR_T_ARG_C,
	VAR_T_ARG_BK,
	VAR_T_ARG_CK,
	VAR_T_ARG_C_NUM_CONSTANT,
	VAR_T_ARG_C_NEXT_INSTRUCTION,
	VAR_T_ARG_Bx,
	VAR_T_ARG_sBx,
	VAR_T_PC_OFFSET,
	VAR_T_PC_PTR,
	VAR_T_INSTRUCTION,
	VAR_T_NEXT_INSTRUCTION,
	VAR_T_FUNC_STATE_PTR,
	VAR_T_LUA_STATE_PTR,
	VAR_T_K,
	VAR_T_CL,
} val_t;

typedef struct {
	int opcode; /* Lua opcode */
	hint_t hint; /* Specialized version. [0=generic] */
	val_t ret_type; /* return type */
	char *name; /* function name */
	val_t params[10]; /* an 'VOID' type ends the parameter list */
} vm_func_info;

extern const vm_func_info vm_op_functions[];

extern int vm_op_run_count[];

extern void vm_count_OP(const Instruction i);

extern void vm_print_OP(lua_State *L, LClosure *cl, const Instruction i);

extern void vm_next_OP(lua_State *L, LClosure *cl);

extern void vm_OP_MOVE(lua_State *L, int a, int b);

extern void vm_OP_LOADK(lua_State *L, TValue *k, int a, int bx);

extern void vm_OP_LOADBOOL(lua_State *L, int a, int b, int c);

extern void vm_OP_LOADNIL(lua_State *L, int a, int b);

extern void vm_OP_GETUPVAL(lua_State *L, LClosure *cl, int a, int b);

extern void vm_OP_GETGLOBAL(lua_State *L, TValue *k, LClosure *cl, int a, int bx);

extern void vm_OP_GETTABLE(lua_State *L, TValue *k, int a, int b, int c);

extern void vm_OP_SETGLOBAL(lua_State *L, TValue *k, LClosure *cl, int a, int bx);

extern void vm_OP_SETUPVAL(lua_State *L, LClosure *cl, int a, int b);

extern void vm_OP_SETTABLE(lua_State *L, TValue *k, int a, int b, int c);

extern void vm_OP_NEWTABLE(lua_State *L, int a, int b, int c);

extern void vm_OP_SELF(lua_State *L, TValue *k, int a, int b, int c);

extern void vm_OP_ADD(lua_State *L, TValue *k, int a, int b, int c);
extern void vm_OP_ADD_NC(lua_State *L, TValue *k, int a, int b, lua_Number nc, int c);

extern void vm_OP_SUB(lua_State *L, TValue *k, int a, int b, int c);
extern void vm_OP_SUB_NC(lua_State *L, TValue *k, int a, int b, lua_Number nc, int c);

extern void vm_OP_MUL(lua_State *L, TValue *k, int a, int b, int c);
extern void vm_OP_MUL_NC(lua_State *L, TValue *k, int a, int b, lua_Number nc, int c);

extern void vm_OP_DIV(lua_State *L, TValue *k, int a, int b, int c);
extern void vm_OP_DIV_NC(lua_State *L, TValue *k, int a, int b, lua_Number nc, int c);

extern void vm_OP_MOD(lua_State *L, TValue *k, int a, int b, int c);
extern void vm_OP_MOD_NC(lua_State *L, TValue *k, int a, int b, lua_Number nc, int c);

extern void vm_OP_POW(lua_State *L, TValue *k, int a, int b, int c);
extern void vm_OP_POW_NC(lua_State *L, TValue *k, int a, int b, lua_Number nc, int c);

extern void vm_OP_UNM(lua_State *L, int a, int b);

extern void vm_OP_NOT(lua_State *L, int a, int b);

extern void vm_OP_LEN(lua_State *L, int a, int b);

extern void vm_OP_CONCAT(lua_State *L, int a, int b, int c);

extern void vm_OP_JMP(lua_State *L, int sbx);

extern int vm_OP_EQ(lua_State *L, TValue *k, int a, int b, int c);
extern int vm_OP_EQ_NC(lua_State *L, TValue *k, int a, int b, lua_Number nc, int c);

extern int vm_OP_LT(lua_State *L, TValue *k, int a, int b, int c);

extern int vm_OP_LE(lua_State *L, TValue *k, int a, int b, int c);

extern int vm_OP_TEST(lua_State *L, int a, int c);

extern int vm_OP_TESTSET(lua_State *L, int a, int b, int c);

extern int vm_OP_CALL(lua_State *L, int a, int b, int c);

extern int vm_OP_RETURN(lua_State *L, int a, int b);

extern int vm_OP_TAILCALL(lua_State *L, int a, int b, int c);

extern int vm_OP_FORLOOP(lua_State *L, int a, int sbx);

extern int vm_OP_FORPREP(lua_State *L, int a, int sbx);

extern int vm_OP_TFORLOOP(lua_State *L, int a, int c);

extern void vm_OP_SETLIST(lua_State *L, int a, int b, int c, int c_next);

extern void vm_OP_CLOSE(lua_State *L, int a);

extern void vm_OP_CLOSURE(lua_State *L, LClosure *cl, int a, int bx, int pseudo_ops_offset);

extern void vm_OP_VARARG(lua_State *L, LClosure *cl, int a, int b);

extern LClosure *vm_get_current_closure(lua_State *L);

extern TValue *vm_get_current_constants(LClosure *cl);

/*
** some macros for common tasks in `vm_OP_*' functions.
*/

#define runtime_check(L, c) { if (!(c)) return; }

#define RA(i) (base+GETARG_A(i))
/* to be used after possible stack reallocation */
#define RB(i) check_exp(getBMode(GET_OPCODE(i)) == OpArgR, base+GETARG_B(i))
#define RC(i) check_exp(getCMode(GET_OPCODE(i)) == OpArgR, base+GETARG_C(i))
#define RKB(i)  check_exp(getBMode(GET_OPCODE(i)) == OpArgK, \
  ISK(GETARG_B(i)) ? k+INDEXK(GETARG_B(i)) : base+GETARG_B(i))
#define RKC(i)  check_exp(getCMode(GET_OPCODE(i)) == OpArgK, \
  ISK(GETARG_C(i)) ? k+INDEXK(GETARG_C(i)) : base+GETARG_C(i))
#define KBx(i)  check_exp(getBMode(GET_OPCODE(i)) == OpArgK, k+GETARG_Bx(i))

#define RK(arg)  (ISK(arg) ? k+INDEXK(arg) : base+arg)

#ifndef LUA_NODEBUG
#define dojump(i)  {(L->savedpc) += (i); luai_threadyield(L);}
#define skip_op()   (L->savedpc)++;
#else
#define dojump(i)  {luai_threadyield(L);}
#define skip_op()
#endif


#define Protect(x)  { {x;}; base = L->base; }

#define arith_op(op,tm) { \
        TValue *ra = base + a; \
        TValue *rb = RK(b); \
        TValue *rc = RK(c); \
        if (ttisnumber(rb) && ttisnumber(rc)) { \
          lua_Number nb = nvalue(rb), nc = nvalue(rc); \
          setnvalue(ra, op(nb, nc)); \
        } \
        else \
          luaV_arith(L, ra, rb, rc, tm); \
      }

#define arith_op_nc(op,tm) { \
        TValue *ra = base + a; \
        TValue *rb = RK(b); \
        if (ttisnumber(rb)) { \
          lua_Number nb = nvalue(rb); \
          setnvalue(ra, op(nb, nc)); \
        } \
        else \
          luaV_arith(L, ra, rb, RK(c), tm); \
      }


#ifdef __cplusplus
}
#endif

#endif

