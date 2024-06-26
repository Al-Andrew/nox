#include "object.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"
#include "vm.h"
#include "memory.h"

Clox_Object* Clox_Object_Allocate(Clox_VM* vm, Clox_Object_Type type, uint32_t size) {
    CLOX_DEV_ASSERT(size >= sizeof(Clox_Object));
    Clox_Object* retval = (Clox_Object*)reallocate(NULL, 0, size);
    retval->type = type;

    retval->next_object = vm->objects;
    vm->objects = retval; 
    
    return retval;
}

void Clox_Object_Deallocate(Clox_VM* vm, Clox_Object* object) {
    (void)vm; //NOTE(AAL): wtf?????? how is this unused
    
    switch (object->type) {
        case CLOX_OBJECT_TYPE_STRING: /* fallthrough */
        case CLOX_OBJECT_TYPE_NATIVE: /* fallthrough */
        case CLOX_OBJECT_TYPE_CLOSURE: /* fallthrough */
        case CLOX_OBJECT_TYPE_UPVALUE: {
            deallocate(object);
        } break;
        case CLOX_OBJECT_TYPE_FUNCTION: {
            Clox_Function* function = (Clox_Function*)object;
            Clox_Chunk_Delete(&function->chunk);
            deallocate(object);
        } break;
    }
}

static uint32_t fnv_1a(const char* key, uint32_t length) {
    uint32_t hash = 2166136261u;
    for (uint32_t i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

Clox_String* Clox_String_Create(Clox_VM* vm, const char* string, uint32_t len) {
    // Check if the string is already interned
    uint32_t hash = fnv_1a(string, len);
    Clox_Hash_Table_Entry* interned = Clox_Hash_Table_Get_Raw(&vm->strings, string, len, hash);
    if(interned != NULL) {
        return interned->key;
    }

    // Allocate a new one
    Clox_String* retval = (Clox_String*)Clox_Object_Allocate(vm, CLOX_OBJECT_TYPE_STRING, sizeof(Clox_String) + len + 1);
    retval->hash = hash;
    retval->length = len;
    memcpy(retval->characters, string, len);
    retval->characters[len] = '\0';
    Clox_Hash_Table_Set(&vm->strings, retval, CLOX_VALUE_NIL);

    return retval;
}

void Clox_Object_Print(Clox_Object const* const object) {
    switch (object->type) {
        case CLOX_OBJECT_TYPE_STRING: {
            Clox_String* string = (Clox_String*)object;
            printf("\"%.*s\"", string->length, string->characters);
        } break;
        case CLOX_OBJECT_TYPE_FUNCTION: {
            Clox_Function* fn = (Clox_Function*)object;
            if (fn->name == NULL) {
                printf("<script>");
                return;
            }
            printf("<fn %.*s>", fn->name->length, fn->name->characters);
        } break;
        case CLOX_OBJECT_TYPE_NATIVE: {
            Clox_Native* native = (Clox_Native*)object;
            printf("<native 0x%lX>", (uintptr_t)native->function);
        } break;
        case CLOX_OBJECT_TYPE_CLOSURE: {
            Clox_Closure* fn = (Clox_Closure*)object;
            if (fn->function->name == NULL) {
                printf("<closure>");
                return;
            }
            printf("<closure %.*s>", fn->function->name->length, fn->function->name->characters);
        } break;
        case CLOX_OBJECT_TYPE_UPVALUE: {
            printf("upvalue");
        } break;
        }
}

Clox_Function* Clox_Function_Create_Empty(Clox_VM* vm) {
    Clox_Function* function = (Clox_Function*)Clox_Object_Allocate(vm, CLOX_OBJECT_TYPE_FUNCTION,sizeof(Clox_Function));
    function->arity = 0;
    function->upvalue_count = 0;
    function->name = NULL;
    function->chunk = Clox_Chunk_New_Empty();
    return function;
}

Clox_Native* Clox_Native_Create(Clox_VM* vm, Clox_Native_Fn lambda) {
    Clox_Native* native = (Clox_Native*)Clox_Object_Allocate(vm, CLOX_OBJECT_TYPE_NATIVE, sizeof(Clox_Native));
    native->function = lambda;

    return native;
}

Clox_Closure* Clox_Closure_Create(Clox_VM* vm, Clox_Function* function) {

    Clox_Closure* closure = (Clox_Closure*)Clox_Object_Allocate(
        vm,
        CLOX_OBJECT_TYPE_CLOSURE,
        (uint32_t)(sizeof(Clox_Closure) + sizeof(Clox_UpvalueObj*) * (uint64_t)function->upvalue_count)
    );
    closure->function = function;
    closure->upvalue_count = function->upvalue_count;

    for (int i = 0; i < function->upvalue_count; i++) {
        closure->upvalues[i] = NULL;
    }
    return closure;
}

Clox_UpvalueObj* Clox_UpvalueObj_Create(Clox_VM* vm, Clox_Value* slot) {
    
    Clox_UpvalueObj* prevUpvalue = NULL;
    Clox_UpvalueObj* upvalue = vm->open_upvalues;
    while (upvalue != NULL && upvalue->location > slot) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == slot) {
        return upvalue;
    }

    Clox_UpvalueObj* created_upvalue = (Clox_UpvalueObj*)Clox_Object_Allocate(vm, CLOX_OBJECT_TYPE_UPVALUE, sizeof(Clox_UpvalueObj));
    created_upvalue->location = slot;
    created_upvalue->next = NULL;
    created_upvalue->next = upvalue;
    created_upvalue->closed = CLOX_VALUE_NIL;

    if (prevUpvalue == NULL) {
        vm->open_upvalues = created_upvalue;
    } else {
        prevUpvalue->next = created_upvalue;
    }

    return created_upvalue;
}

Clox_UpvalueObj* Clox_Closure_Capture_Upvalue(Clox_VM* vm, Clox_Value* value) {
    Clox_UpvalueObj* captured = Clox_UpvalueObj_Create(vm, value);
    return captured;
}
