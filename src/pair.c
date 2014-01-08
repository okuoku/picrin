#include <stdarg.h>

#include "picrin.h"
#include "picrin/pair.h"

pic_value
pic_cons(pic_state *pic, pic_value car, pic_value cdr)
{
  struct pic_pair *pair;

  pair = (struct pic_pair *)pic_obj_alloc(pic, sizeof(struct pic_pair), PIC_TT_PAIR);
  pair->car = car;
  pair->cdr = cdr;

  return pic_obj_value(pair);
}

pic_value
pic_car(pic_state *pic, pic_value obj)
{
  struct pic_pair *pair;

  if (! pic_pair_p(obj)) {
    pic_error(pic, "pair required");
  }
  pair = pic_pair_ptr(obj);

  return pair->car;
}

pic_value
pic_cdr(pic_state *pic, pic_value obj)
{
  struct pic_pair *pair;

  if (! pic_pair_p(obj)) {
    pic_error(pic, "pair required");
  }
  pair = pic_pair_ptr(obj);

  return pair->cdr;
}

bool
pic_list_p(pic_state *pic, pic_value obj)
{
  while (pic_pair_p(obj))
    obj = pic_pair_ptr(obj)->cdr;

  return pic_nil_p(obj);
}

pic_value
pic_list(pic_state *pic, size_t c, ...)
{
  va_list ap;
  pic_value v;

  va_start(ap, c);

  v = pic_nil_value();
  while (c--) {
    v = pic_cons(pic, va_arg(ap, pic_value), v);
  }

  va_end(ap);
  return pic_reverse(pic, v);
}

pic_value
pic_list_from_array(pic_state *pic, size_t c, pic_value *vs)
{
  pic_value v;
  int i;

  v = pic_nil_value();
  for (i = 0; i < c; ++i) {
    v = pic_cons(pic, vs[i], v);
  }
  return pic_reverse(pic, v);
}

int
pic_length(pic_state *pic, pic_value obj)
{
  int c = 0;

  while (! pic_nil_p(obj)) {
    obj = pic_cdr(pic, obj);
    ++c;
  }

  return c;
}

pic_value
pic_reverse(pic_state *pic, pic_value list)
{
  int ai = pic_gc_arena_preserve(pic);
  pic_value v, acc;

  acc = pic_nil_value();
  for (v = list; ! pic_nil_p(v); v = pic_cdr(pic ,v)) {
    acc = pic_cons(pic, pic_car(pic, v), acc);

    pic_gc_arena_restore(pic, ai);
    pic_gc_protect(pic, acc);
    pic_gc_protect(pic, v);
  }
  return acc;
}

pic_value
pic_assq(pic_state *pic, pic_value key, pic_value assoc)
{
  pic_value cell;

 enter:

  if (pic_nil_p(assoc))
    return pic_false_value();

  cell = pic_car(pic, assoc);
  if (pic_eq_p(key, pic_car(pic, cell)))
    return cell;

  assoc = pic_cdr(pic, assoc);
  goto enter;
}

pic_value
pic_assoc(pic_state *pic, pic_value key, pic_value assoc)
{
  pic_value cell;

 enter:

  if (pic_nil_p(assoc))
    return pic_false_value();

  cell = pic_car(pic, assoc);
  if (pic_equal_p(pic, key, pic_car(pic, cell)))
    return cell;

  assoc = pic_cdr(pic, assoc);
  goto enter;
}

pic_value
pic_acons(pic_state *pic, pic_value key, pic_value val, pic_value assoc)
{
  return pic_cons(pic, pic_cons(pic, key, val), assoc);
}

pic_value
pic_caar(pic_state *pic, pic_value v)
{
  return pic_car(pic, pic_car(pic, v));
}

pic_value
pic_cadr(pic_state *pic, pic_value v)
{
  return pic_car(pic, pic_cdr(pic, v));
}

pic_value
pic_cdar(pic_state *pic, pic_value v)
{
  return pic_cdr(pic, pic_car(pic, v));
}

pic_value
pic_cddr(pic_state *pic, pic_value v)
{
  return pic_cdr(pic, pic_cdr(pic, v));
}

static pic_value
pic_pair_pair_p(pic_state *pic)
{
  pic_value v;

  pic_get_args(pic, "o", &v);

  return pic_bool_value(pic_pair_p(v));
}

static pic_value
pic_pair_car(pic_state *pic)
{
  pic_value v;

  pic_get_args(pic, "o", &v);

  return pic_car(pic, v);
}

static pic_value
pic_pair_cdr(pic_state *pic)
{
  pic_value v;

  pic_get_args(pic, "o", &v);

  return pic_cdr(pic, v);
}

static pic_value
pic_pair_null_p(pic_state *pic)
{
  pic_value v;

  pic_get_args(pic, "o", &v);

  return pic_bool_value(pic_nil_p(v));
}

static pic_value
pic_pair_cons(pic_state *pic)
{
  pic_value v,w;

  pic_get_args(pic, "oo", &v, &w);

  return pic_cons(pic, v, w);
}

static pic_value
pic_pair_set_car(pic_state *pic)
{
  pic_value v,w;

  pic_get_args(pic, "oo", &v, &w);

  if (! pic_pair_p(v))
    pic_error(pic, "pair expected");

  pic_pair_ptr(v)->car = w;
  return pic_none_value();
}

static pic_value
pic_pair_set_cdr(pic_state *pic)
{
  pic_value v,w;

  pic_get_args(pic, "oo", &v, &w);

  if (! pic_pair_p(v))
    pic_error(pic, "pair expected");

  pic_pair_ptr(v)->cdr = w;
  return pic_none_value();
}

void
pic_init_pair(pic_state *pic)
{
  pic_defun(pic, "pair?", pic_pair_pair_p);
  pic_defun(pic, "car", pic_pair_car);
  pic_defun(pic, "cdr", pic_pair_cdr);
  pic_defun(pic, "null?", pic_pair_null_p);
  pic_defun(pic, "cons", pic_pair_cons);
  pic_defun(pic, "set-car!", pic_pair_set_car);
  pic_defun(pic, "set-cdr!", pic_pair_set_cdr);
}
