/**
 * See Copyright Notice in picrin.h
 */

#include "picrin.h"

static void
pic_init_features(pic_state *pic)
{
  pic_add_feature(pic, "picrin");

#if __STDC_IEC_559__
  pic_add_feature(pic, "ieee-float");
#endif

#if _POSIX_SOURCE
  pic_add_feature(pic, "posix");
#endif

#if _WIN32
  pic_add_feature(pic, "windows");
#endif

#if __unix__
  pic_add_feature(pic, "unix");
#endif
#if __gnu_linux__
  pic_add_feature(pic, "gnu-linux");
#endif
#if __FreeBSD__
  pic_add_feature(pic, "freebsd");
#endif

#if __i386__
  pic_add_feature(pic, "i386");
#elif __x86_64__
  pic_add_feature(pic, "x86-64");
#elif __ppc__
  pic_add_feature(pic, "ppc");
#elif __sparc__
  pic_add_feature(pic, "sparc");
#endif

#if __ILP32__
  pic_add_feature(pic, "ilp32");
#elif __LP64__
  pic_add_feature(pic, "lp64");
#endif

#if defined(__BYTE_ORDER__)
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  pic_add_feature(pic, "little-endian");
# elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  pic_add_feature(pic, "big-endian");
# endif
#else
# if __LITTLE_ENDIAN__
  pic_add_feature(pic, "little-endian");
# elif __BIG_ENDIAN__
  pic_add_feature(pic, "big-endian");
# endif
#endif
}

void
pic_add_feature(pic_state *pic, const char *feature)
{
  pic_push(pic, pic_obj_value(pic_intern_cstr(pic, feature)), pic->features);
}

static pic_value
pic_features(pic_state *pic)
{
  pic_get_args(pic, "");

  return pic->features;
}

#define import_builtin_syntax(name) do {                                \
    pic_sym *nick, *real;                                               \
    nick = pic_intern_lit(pic, "builtin:" name);                        \
    real = pic_intern_lit(pic, name);                                   \
    pic_put_identifier(pic, (pic_id *)nick, real, pic->lib->env);       \
  } while (0)

#define declare_vm_procedure(name) do {                                 \
    pic_sym *sym;                                                       \
    sym = pic_intern_lit(pic, name);                                    \
    pic_put_identifier(pic, (pic_id *)sym, sym, pic->lib->env);         \
  } while (0)

void pic_init_bool(pic_state *);
void pic_init_pair(pic_state *);
void pic_init_port(pic_state *);
void pic_init_number(pic_state *);
void pic_init_proc(pic_state *);
void pic_init_symbol(pic_state *);
void pic_init_vector(pic_state *);
void pic_init_blob(pic_state *);
void pic_init_cont(pic_state *);
void pic_init_char(pic_state *);
void pic_init_error(pic_state *);
void pic_init_str(pic_state *);
void pic_init_var(pic_state *);
void pic_init_write(pic_state *);
void pic_init_read(pic_state *);
void pic_init_dict(pic_state *);
void pic_init_record(pic_state *);
void pic_init_eval(pic_state *);
void pic_init_lib(pic_state *);
void pic_init_reg(pic_state *);

extern const char pic_boot[][80];

static void
pic_init_core(pic_state *pic)
{
  struct pic_box *pic_vm_gref_slot(pic_state *, pic_sym *);

  pic_init_features(pic);

  pic_deflibrary (pic, "(picrin base)") {
    size_t ai = pic_gc_arena_preserve(pic);

#define DONE pic_gc_arena_restore(pic, ai);

    import_builtin_syntax("define");
    import_builtin_syntax("set!");
    import_builtin_syntax("quote");
    import_builtin_syntax("lambda");
    import_builtin_syntax("if");
    import_builtin_syntax("begin");
    import_builtin_syntax("define-macro");

    declare_vm_procedure("cons");
    declare_vm_procedure("car");
    declare_vm_procedure("cdr");
    declare_vm_procedure("null?");
    declare_vm_procedure("symbol?");
    declare_vm_procedure("pair?");
    declare_vm_procedure("+");
    declare_vm_procedure("-");
    declare_vm_procedure("*");
    declare_vm_procedure("/");
    declare_vm_procedure("=");
    declare_vm_procedure("<");
    declare_vm_procedure(">");
    declare_vm_procedure("<=");
    declare_vm_procedure(">=");
    declare_vm_procedure("not");

    DONE;

    pic_init_bool(pic); DONE;
    pic_init_pair(pic); DONE;
    pic_init_port(pic); DONE;
    pic_init_number(pic); DONE;
    pic_init_proc(pic); DONE;
    pic_init_symbol(pic); DONE;
    pic_init_vector(pic); DONE;
    pic_init_blob(pic); DONE;
    pic_init_cont(pic); DONE;
    pic_init_char(pic); DONE;
    pic_init_error(pic); DONE;
    pic_init_str(pic); DONE;
    pic_init_var(pic); DONE;
    pic_init_write(pic); DONE;
    pic_init_read(pic); DONE;
    pic_init_dict(pic); DONE;
    pic_init_record(pic); DONE;
    pic_init_eval(pic); DONE;
    pic_init_lib(pic); DONE;
    pic_init_reg(pic); DONE;

    pic_defun(pic, "features", pic_features);

    pic_try {
      pic_load_cstr(pic, &pic_boot[0][0]);
    }
    pic_catch {
      pic_print_backtrace(pic, xstdout);
      pic_panic(pic, "");
    }
  }
}

pic_state *
pic_open(pic_allocf allocf, void *userdata)
{
  struct pic_port *pic_make_standard_port(pic_state *, xFILE *, short);
  char t;

  pic_state *pic;
  size_t ai;

  pic = allocf(userdata, NULL, sizeof(pic_state));

  if (! pic) {
    goto EXIT_PIC;
  }

  /* allocator */
  pic->allocf = allocf;

  /* user data */
  pic->userdata = userdata;

  /* turn off GC */
  pic->gc_enable = false;

  /* continuation chain */
  pic->cc = NULL;
  pic->ccnt = 0;

  /* root block */
  pic->cp = NULL;

  /* prepare VM stack */
  pic->stbase = pic->sp = allocf(userdata, NULL, PIC_STACK_SIZE * sizeof(pic_value));
  pic->stend = pic->stbase + PIC_STACK_SIZE;

  if (! pic->sp) {
    goto EXIT_SP;
  }

  /* callinfo */
  pic->cibase = pic->ci = allocf(userdata, NULL, PIC_STACK_SIZE * sizeof(pic_callinfo));
  pic->ciend = pic->cibase + PIC_STACK_SIZE;

  if (! pic->ci) {
    goto EXIT_CI;
  }

  /* exception handler */
  pic->xpbase = pic->xp = allocf(userdata, NULL, PIC_RESCUE_SIZE * sizeof(struct pic_proc *));
  pic->xpend = pic->xpbase + PIC_RESCUE_SIZE;

  if (! pic->xp) {
    goto EXIT_XP;
  }

  /* GC arena */
  pic->arena = allocf(userdata, NULL, PIC_ARENA_SIZE * sizeof(struct pic_object *));
  pic->arena_size = PIC_ARENA_SIZE;
  pic->arena_idx = 0;

  if (! pic->arena) {
    goto EXIT_ARENA;
  }

  /* memory heap */
  pic->heap = pic_heap_open(pic);

  /* symbol table */
  kh_init(s, &pic->syms);

  /* unique symbol count */
  pic->ucnt = 0;

  /* global variables */
  pic->globals = NULL;

  /* macros */
  pic->macros = NULL;

  /* features */
  pic->features = pic_nil_value();

  /* libraries */
  pic->libs = pic_nil_value();
  pic->lib = NULL;

  /* ireps */
  pic->ireps.next = &pic->ireps;
  pic->ireps.prev = &pic->ireps;

  /* raised error object */
  pic->err = pic_invalid_value();

  /* file pool */
  memset(pic->files, 0, sizeof pic->files);

  /* parameter table */
  pic->ptable = pic_nil_value();

  /* native stack marker */
  pic->native_stack_start = &t;

  ai = pic_gc_arena_preserve(pic);

#define S(slot,name) pic->slot = pic_intern_lit(pic, name)

  S(sDEFINE, "define");
  S(sDEFINE_MACRO, "define-macro");
  S(sLAMBDA, "lambda");
  S(sIF, "if");
  S(sBEGIN, "begin");
  S(sSETBANG, "set!");
  S(sQUOTE, "quote");
  S(sQUASIQUOTE, "quasiquote");
  S(sUNQUOTE, "unquote");
  S(sUNQUOTE_SPLICING, "unquote-splicing");
  S(sSYNTAX_QUOTE, "syntax-quote");
  S(sSYNTAX_QUASIQUOTE, "syntax-quasiquote");
  S(sSYNTAX_UNQUOTE, "syntax-unquote");
  S(sSYNTAX_UNQUOTE_SPLICING, "syntax-unquote-splicing");
  S(sIMPORT, "import");
  S(sEXPORT, "export");
  S(sDEFINE_LIBRARY, "define-library");
  S(sCOND_EXPAND, "cond-expand");

  S(sCONS, "cons");
  S(sCAR, "car");
  S(sCDR, "cdr");
  S(sNILP, "null?");
  S(sSYMBOLP, "symbol?");
  S(sPAIRP, "pair?");
  S(sADD, "+");
  S(sSUB, "-");
  S(sMUL, "*");
  S(sDIV, "/");
  S(sEQ, "=");
  S(sLT, "<");
  S(sLE, "<=");
  S(sGT, ">");
  S(sGE, ">=");
  S(sNOT, "not");

  pic_gc_arena_restore(pic, ai);

  /* root tables */
  pic->globals = pic_make_reg(pic);
  pic->macros = pic_make_reg(pic);

  /* root block */
  pic->cp = (pic_checkpoint *)pic_obj_alloc(pic, sizeof(pic_checkpoint), PIC_TT_CP);
  pic->cp->prev = NULL;
  pic->cp->depth = 0;
  pic->cp->in = pic->cp->out = NULL;

  /* reader */
  pic_reader_init(pic);

  /* parameter table */
  pic->ptable = pic_cons(pic, pic_obj_value(pic_make_reg(pic)), pic->ptable);

  /* standard libraries */
  pic->PICRIN_BASE = pic_make_library(pic, pic_read_cstr(pic, "(picrin base)"));
  pic->PICRIN_USER = pic_make_library(pic, pic_read_cstr(pic, "(picrin user)"));
  pic->lib = pic->PICRIN_USER;
  pic->prev_lib = NULL;

  pic_gc_arena_restore(pic, ai);

  /* turn on GC */
  pic->gc_enable = true;

  pic_init_core(pic);

  pic_gc_arena_restore(pic, ai);

  return pic;

 EXIT_ARENA:
  allocf(userdata, pic->xp, 0);
 EXIT_XP:
  allocf(userdata, pic->ci, 0);
 EXIT_CI:
  allocf(userdata, pic->sp, 0);
 EXIT_SP:
  allocf(userdata, pic, 0);
 EXIT_PIC:
  return NULL;
}

void
pic_close(pic_state *pic)
{
  khash_t(s) *h = &pic->syms;
  pic_allocf allocf = pic->allocf;

  /* clear out root objects */
  pic->sp = pic->stbase;
  pic->ci = pic->cibase;
  pic->xp = pic->xpbase;
  pic->arena_idx = 0;
  pic->err = pic_invalid_value();
  pic->globals = NULL;
  pic->macros = NULL;
  pic->features = pic_nil_value();
  pic->libs = pic_nil_value();

  /* free all heap objects */
  pic_gc_run(pic);

#if 0
  {
    /* FIXME */
    int i = 0;
    struct pic_list *list;
    for (list = pic->ireps.next; list != &pic->ireps; list = list->next) {
      i++;
    }
    printf("%d\n", i);
  }
#endif

  /* flush all xfiles */
  xfflush(pic, NULL);

  /* free heaps */
  pic_heap_close(pic, pic->heap);

  /* free reader struct */
  pic_reader_destroy(pic);

  /* free runtime context */
  allocf(pic->userdata, pic->stbase, 0);
  allocf(pic->userdata, pic->cibase, 0);
  allocf(pic->userdata, pic->xpbase, 0);

  /* free global stacks */
  kh_destroy(s, h);

  /* free GC arena */
  allocf(pic->userdata, pic->arena, 0);

  allocf(pic->userdata, pic, 0);
}
