#ifndef CLOX_VALUE_H_INCLUDED
#define CLOX_VALUE_H_INCLUDED

#include "common.h"

typedef enum {
  CLOX_VALUE_TYPE_NIL,
  CLOX_VALUE_TYPE_BOOL,
  CLOX_VALUE_TYPE_NUMBER,
  CLOX_VALUE_TYPE_OBJECT,
} Clox_Value_Type;

struct Clox_Object;

struct Clox_Value {
  Clox_Value_Type type;
  union {
    bool boolean;
    double number;
    struct Clox_Object* object;
  } value; 
};


#define CLOX_VALUE_IS_BOOL(value)    ((value).type == CLOX_VALUE_TYPE_BOOL)
#define CLOX_VALUE_IS_NIL(value)     ((value).type == CLOX_VALUE_TYPE_NIL)
#define CLOX_VALUE_IS_NUMBER(value)  ((value).type == CLOX_VALUE_TYPE_NUMBER)
#define CLOX_VALUE_IS_OBJECT(value)  ((value).type == CLOX_VALUE_TYPE_OBJECT)

#define CLOX_VALUE_BOOL(val)   ((Clox_Value){CLOX_VALUE_TYPE_BOOL, .value.boolean = val  })
#define CLOX_VALUE_NIL           ((Clox_Value){CLOX_VALUE_TYPE_NIL, .value.number = 0})
#define CLOX_VALUE_NUMBER(val) ((Clox_Value){CLOX_VALUE_TYPE_NUMBER, .value.number = val})
#define CLOX_VALUE_OBJECT(obj)   ((Clox_Value){CLOX_VALUE_TYPE_OBJECT, .value.object = (Clox_Object*)(obj)})

#endif // CLOX_VALUE_H_INCLUDED