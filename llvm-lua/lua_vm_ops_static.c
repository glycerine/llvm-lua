/*
** See Copyright Notice in lua.h
*/

/*
 * lua_vm_ops.c -- Lua ops functions for use by LLVM IR gen.
 *
 * Most of this file was copied from Lua's lvm.c
 */

#include "lua_vm_ops.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"
#include <stdio.h>
#include <assert.h>

#include "llvm_compiler.h"

const vm_func_info vm_op_functions[] = {
  { OP_MOVE, HINT_T_NONE, VAR_T_VOID, "vm_OP_MOVE",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_VOID},
  },
  { OP_LOADK, HINT_T_NONE, VAR_T_VOID, "vm_OP_LOADK",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_Bx, VAR_T_VOID},
  },
  { OP_LOADBOOL, HINT_T_NONE, VAR_T_VOID, "vm_OP_LOADBOOL",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_LOADNIL, HINT_T_NONE, VAR_T_VOID, "vm_OP_LOADNIL",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_VOID},
  },
  { OP_GETUPVAL, HINT_T_NONE, VAR_T_VOID, "vm_OP_GETUPVAL",
    {VAR_T_LUA_STATE_PTR, VAR_T_CL, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_VOID},
  },
  { OP_GETGLOBAL, HINT_T_NONE, VAR_T_VOID, "vm_OP_GETGLOBAL",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_CL, VAR_T_ARG_A, VAR_T_ARG_Bx, VAR_T_VOID},
  },
  { OP_GETTABLE, HINT_T_NONE, VAR_T_VOID, "vm_OP_GETTABLE",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_SETGLOBAL, HINT_T_NONE, VAR_T_VOID, "vm_OP_SETGLOBAL",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_CL, VAR_T_ARG_A, VAR_T_ARG_Bx, VAR_T_VOID},
  },
  { OP_SETUPVAL, HINT_T_NONE, VAR_T_VOID, "vm_OP_SETUPVAL",
    {VAR_T_LUA_STATE_PTR, VAR_T_CL, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_VOID},
  },
  { OP_SETTABLE, HINT_T_NONE, VAR_T_VOID, "vm_OP_SETTABLE",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_NEWTABLE, HINT_T_NONE, VAR_T_VOID, "vm_OP_NEWTABLE",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_SELF, HINT_T_NONE, VAR_T_VOID, "vm_OP_SELF",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_ADD, HINT_T_NONE, VAR_T_VOID, "vm_OP_ADD",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_ADD, HINT_T_C_NUM_CONSTANT, VAR_T_VOID, "vm_OP_ADD_NC",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C_NUM_CONSTANT, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_SUB, HINT_T_NONE, VAR_T_VOID, "vm_OP_SUB",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_SUB, HINT_T_C_NUM_CONSTANT, VAR_T_VOID, "vm_OP_SUB_NC",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C_NUM_CONSTANT, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_MUL, HINT_T_NONE, VAR_T_VOID, "vm_OP_MUL",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_MUL, HINT_T_C_NUM_CONSTANT, VAR_T_VOID, "vm_OP_MUL_NC",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C_NUM_CONSTANT, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_DIV, HINT_T_NONE, VAR_T_VOID, "vm_OP_DIV",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_DIV, HINT_T_C_NUM_CONSTANT, VAR_T_VOID, "vm_OP_DIV_NC",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C_NUM_CONSTANT, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_MOD, HINT_T_NONE, VAR_T_VOID, "vm_OP_MOD",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_MOD, HINT_T_C_NUM_CONSTANT, VAR_T_VOID, "vm_OP_MOD_NC",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C_NUM_CONSTANT, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_POW, HINT_T_NONE, VAR_T_VOID, "vm_OP_POW",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_POW, HINT_T_C_NUM_CONSTANT, VAR_T_VOID, "vm_OP_POW_NC",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C_NUM_CONSTANT, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_UNM, HINT_T_NONE, VAR_T_VOID, "vm_OP_UNM",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_VOID},
  },
  { OP_NOT, HINT_T_NONE, VAR_T_VOID, "vm_OP_NOT",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_VOID},
  },
  { OP_LEN, HINT_T_NONE, VAR_T_VOID, "vm_OP_LEN",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_VOID},
  },
  { OP_CONCAT, HINT_T_NONE, VAR_T_VOID, "vm_OP_CONCAT",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_JMP, HINT_T_NONE, VAR_T_VOID, "vm_OP_JMP",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_sBx, VAR_T_VOID},
  },
  { OP_EQ, HINT_T_NONE, VAR_T_INT, "vm_OP_EQ",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_EQ, HINT_T_C_NUM_CONSTANT, VAR_T_VOID, "vm_OP_EQ_NC",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C_NUM_CONSTANT, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_LT, HINT_T_NONE, VAR_T_INT, "vm_OP_LT",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_LE, HINT_T_NONE, VAR_T_INT, "vm_OP_LE",
    {VAR_T_LUA_STATE_PTR, VAR_T_K, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_TEST, HINT_T_NONE, VAR_T_INT, "vm_OP_TEST",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_TESTSET, HINT_T_NONE, VAR_T_INT, "vm_OP_TESTSET",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_CALL, HINT_T_NONE, VAR_T_INT, "vm_OP_CALL",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_TAILCALL, HINT_T_NONE, VAR_T_INT, "vm_OP_TAILCALL",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_RETURN, HINT_T_NONE, VAR_T_INT, "vm_OP_RETURN",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_VOID},
  },
  { OP_FORLOOP, HINT_T_NONE, VAR_T_INT, "vm_OP_FORLOOP",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_sBx, VAR_T_VOID},
  },
  { OP_FORPREP, HINT_T_NONE, VAR_T_INT, "vm_OP_FORPREP",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_sBx, VAR_T_VOID},
  },
  { OP_TFORLOOP, HINT_T_NONE, VAR_T_INT, "vm_OP_TFORLOOP",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_SETLIST, HINT_T_NONE, VAR_T_VOID, "vm_OP_SETLIST",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_ARG_C_NEXT_INSTRUCTION, VAR_T_ARG_C, VAR_T_VOID},
  },
  { OP_CLOSE, HINT_T_NONE, VAR_T_VOID, "vm_OP_CLOSE",
    {VAR_T_LUA_STATE_PTR, VAR_T_ARG_A, VAR_T_VOID},
  },
  { OP_CLOSURE, HINT_T_NONE, VAR_T_VOID, "vm_OP_CLOSURE",
    {VAR_T_LUA_STATE_PTR, VAR_T_CL, VAR_T_ARG_A, VAR_T_ARG_Bx, VAR_T_PC_OFFSET, VAR_T_VOID},
  },
  { OP_VARARG, HINT_T_NONE, VAR_T_VOID, "vm_OP_VARARG",
    {VAR_T_LUA_STATE_PTR, VAR_T_CL, VAR_T_ARG_A, VAR_T_ARG_B, VAR_T_VOID},
  },
  { -1, HINT_T_NONE, VAR_T_VOID, NULL, {VAR_T_VOID} }
};

int vm_op_run_count[NUM_OPCODES];

void vm_count_OP(const Instruction i) {
	vm_op_run_count[GET_OPCODE(i)]++;
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 */
void vm_print_OP(lua_State *L, LClosure *cl, const Instruction i) {
  int op = GET_OPCODE(i);
#ifndef LUA_NODEBUG
  fprintf(stderr, "%ld: '%s' (%d) = 0x%08X, pc=%p\n", (L->savedpc - cl->p->code),
    luaP_opnames[op], op, i, L->savedpc);
  lua_assert(L->savedpc[0] == i);
#else
  fprintf(stderr, "'%s' (%d) = 0x%08X\n", luaP_opnames[op], op, i);
#endif
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 */
void vm_next_OP(lua_State *L, LClosure *cl) {
#ifndef LUA_NODEBUG
  //vm_print_OP(fs, L->savedpc[0]);
  lua_assert(L->savedpc >= cl->p->code && (L->savedpc < &(cl->p->code[cl->p->sizecode])));
  L->savedpc++;
  if ((L->hookmask & (LUA_MASKLINE | LUA_MASKCOUNT)) &&
      (--L->hookcount == 0 || L->hookmask & LUA_MASKLINE)) {
    luaV_traceexec(L, L->savedpc);
    if (L->status == LUA_YIELD) {  /* did hook yield? */
      L->savedpc = L->savedpc - 1;
      return;
    }
  }
#endif
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 */
int vm_OP_CALL(lua_State *L, int a, int b, int c) {
  TValue *base = L->base;
  TValue *ra=base + a;
  int nresults = c - 1;
  int ret;
  if (b != 0) L->top = ra+b;  /* else previous instruction set top */
  ret = luaD_precall(L, ra, nresults);
  switch (ret) {
    case PCRLUA: {
      luaV_execute(L, 1);
      break;
    }
    case PCRC: {
      /* it was a C function (`precall' called it); adjust results */
      if (nresults >= 0) L->top = L->ci->top;
      break;
    }
    default: {
      return PCRYIELD;
    }
  }
  return 0;
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 */
int vm_OP_RETURN(lua_State *L, int a, int b) {
  TValue *base = L->base;
  TValue *ra = base + a;
  if (b != 0) L->top = ra+b-1;
  if (L->openupval) luaF_close(L, base);
  b = luaD_poscall(L, ra);
  return PCRC;
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 */
static int vm_OP_TAILCALL_lua(lua_State *L, int a, int b, int c) {
  TValue *base = L->base;
  TValue *ra = base + a;
  int ret;
  if (b != 0) L->top = ra+b;  /* else previous instruction set top */
  lua_assert(c - 1 == LUA_MULTRET);

  ret = luaD_precall(L, ra, LUA_MULTRET);
  lua_assert(ret == PCRLUA);
  /* tail call: put new frame in place of previous one */
  CallInfo *ci = L->ci - 1;  /* previous frame */
  int aux;
  StkId func = ci->func;
  StkId pfunc = (ci+1)->func;  /* previous function index */
  if (L->openupval) luaF_close(L, ci->base);
  L->base = ci->base = ci->func + ((ci+1)->base - pfunc);
  for (aux = 0; pfunc+aux < L->top; aux++)  /* move frame down */
    setobjs2s(L, func+aux, pfunc+aux);
  ci->top = L->top = func+aux;  /* correct top */
  lua_assert(L->top == L->base + clvalue(func)->l.p->maxstacksize);
#ifndef LUA_NODEBUG
  ci->savedpc = L->savedpc;
#endif
  ci->tailcalls++;  /* one more call lost */
  L->ci--;  /* remove new frame */
  luaV_execute(L, 1);
  return PCRC;
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 */
int vm_OP_TAILCALL(lua_State *L, int a, int b, int c) {
  TValue *base = L->base;
  TValue *func = base + a;
  Closure *cl;
  CallInfo *ci;
  StkId cfunc;
  Proto *p;
  int aux;
  if (b != 0) L->top = func+b;  /* else previous instruction set top */
  lua_assert(c - 1 == LUA_MULTRET);
  if (!ttisfunction(func)) /* `func' is not a function? */
    func = luaD_tryfuncTM(L, func);  /* check the `function' tag method */
  cl = clvalue(func);

  if(cl_isLua(cl)) {
    /* check if Lua function was compiled. */
    p = cl->l.p;
    if(p->jit_func == NULL) {
      llvm_compiler_compile(p, 1);
      if(p->jit_func == NULL) {
        /* Can't tailcall into non-compiled lua functions. YET! */
        return vm_OP_TAILCALL_lua(L, a, b, c);
      }
    }
  }
  /* clean up current frame to prepare to tailcall into next function. */
  ci = L->ci;
  cfunc = ci->func; /* current function index */
  if (L->openupval) luaF_close(L, ci->base);
  L->base = cfunc + 1;
  for (aux = 0; func+aux < L->top; aux++)  /* move frame down */
    setobjs2s(L, cfunc+aux, func+aux);
  L->top = cfunc+aux;
  func = cfunc;
  //ci->tailcalls++;  /* one more call lost */
  L->ci--;  /* remove new frame */
  L->savedpc = L->ci->savedpc;
  /* unwind stack back to luaD_precall */
  return PCRTAILCALL;
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 *
 * Notes: split function into two copies, one with number checks + (init - step) + jmp,
 * and the other with the same number checks + slow error throwing code.
 */
int vm_OP_FORPREP(lua_State *L, int a, int sbx) {
  TValue *base = L->base;
  TValue *ra = base + a;
  const TValue *init = ra;
  const TValue *plimit = ra+1;
  const TValue *pstep = ra+2;
  if (!tonumber(init, ra))
    luaG_runerror(L, LUA_QL("for") " initial value must be a number");
  else if (!tonumber(plimit, ra+1))
    luaG_runerror(L, LUA_QL("for") " limit must be a number");
  else if (!tonumber(pstep, ra+2))
    luaG_runerror(L, LUA_QL("for") " step must be a number");
  setnvalue(ra, luai_numsub(nvalue(ra), nvalue(pstep)));
  dojump(sbx);
  return 1;
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 */
int vm_OP_TFORLOOP(lua_State *L, int a, int c) {
  TValue *base = L->base;
  TValue *ra = base + a;
  StkId cb = ra + 3;  /* call base */
  setobjs2s(L, cb+2, ra+2);
  setobjs2s(L, cb+1, ra+1);
  setobjs2s(L, cb, ra);
  L->top = cb+3;  /* func. + 2 args (state and index) */
  Protect(luaD_call(L, cb, c));
  L->top = L->ci->top;
  cb = base + a + 3;  /* previous call may change the stack */
  if (!ttisnil(cb)) {  /* continue loop? */
    setobjs2s(L, cb-1, cb);  /* save control variable */
    dojump(GETARG_sBx(*L->savedpc));
    skip_op();
    return 1;
  }
  skip_op();
  return 0;
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 */
void vm_OP_SETLIST(lua_State *L, int a, int b, int c, int c_next) {
  TValue *base = L->base;
  TValue *ra = base + a;
  int last;
  Table *h;
#ifndef LUA_NODEBUG
  if(c_next == 0) L->savedpc++;
#endif
  if (b == 0) {
    b = cast_int(L->top - ra) - 1;
    L->top = L->ci->top;
  }
  runtime_check(L, ttistable(ra));
  h = hvalue(ra);
  last = ((c-1)*LFIELDS_PER_FLUSH) + b;
  if (last > h->sizearray)  /* needs more space? */
    luaH_resizearray(L, h, last);  /* pre-alloc it at once */
  for (; b > 0; b--) {
    TValue *val = ra+b;
    setobj2t(L, luaH_setnum(L, h, last--), val);
    luaC_barriert(L, h, val);
  }
}

/*
 * TODO: move this function outside of lua_vm_ops.c
 */
void vm_OP_CLOSURE(lua_State *L, LClosure *cl, int a, int bx, int pseudo_ops_offset) {
  TValue *base = L->base;
  const Instruction *pc;
  TValue *ra = base + a;
  Proto *p;
  Closure *ncl;
  int nup, j;

  p = cl->p->p[bx];
  pc=cl->p->code + pseudo_ops_offset;
#ifndef LUA_NODEBUG
  lua_assert(L->savedpc == pc);
#endif
  nup = p->nups;
  ncl = luaF_newLclosure(L, nup, cl->env);
  setclvalue(L, ra, ncl);
  ncl->l.p = p;
  for (j=0; j<nup; j++, pc++) {
    if (GET_OPCODE(*pc) == OP_GETUPVAL)
      ncl->l.upvals[j] = cl->upvals[GETARG_B(*pc)];
    else {
      lua_assert(GET_OPCODE(*pc) == OP_MOVE);
      ncl->l.upvals[j] = luaF_findupval(L, base + GETARG_B(*pc));
    }
  }
#ifndef LUA_NODEBUG
  L->savedpc += nup;
  lua_assert(L->savedpc == pc);
#endif
  luaC_checkGC(L);
}

void vm_OP_VARARG(lua_State *L, LClosure *cl, int a, int b) {
  TValue *base = L->base;
  TValue *ra = base + a;
  int j;
  CallInfo *ci = L->ci;
  int n = cast_int(ci->base - ci->func) - cl->p->numparams - 1;
	b -= 1;
  if (b == LUA_MULTRET) {
    Protect(luaD_checkstack(L, n));
    ra = base + a;  /* previous call may change the stack */
    b = n;
    L->top = ra + n;
  }
  for (j = 0; j < b; j++) {
    if (j < n) {
      setobjs2s(L, ra + j, ci->base - n + j);
    }
    else {
      setnilvalue(ra + j);
    }
  }
}

