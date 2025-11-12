use std::io;

// import C headers into a C namespace
use { "test.h", "test2.h" } as c;
// use { test.hpp, test2.hpp } as cpp;

// c::some_fn();
// cpp::some_temlpate<i32>(1);
//

/*
 * Types:
 *    i8, i16, i32, i64, i128
 *    u8, u16, u32, u64, u128
 *        f16, f32, f64, f128
 *    bool
 *    isize, usize,
 *    type, // generic anytype
 *    cstr <-- null terminated, str 
 *
 *    T[..] // Slice
 *    T[]   // Array
 *    T*    // Reference (cant be null)
 *    T*?   // Pointer (can be null, hence the optional)
 *    T?    // Optional
 *    (T, ...) // tuple
 *
    // comptime-only typeinfo schema
    comptime struct typeinfo {
        // stable identifier (hash or interned)
        id: u128;

        // canonical names and source locations
        canonical_name: cstr;   // e.g. "module::Type<T>"
        short_name: cstr;       // e.g. "Type"
        module_path: cstr;      // e.g. "module::submodule"
        var_name: cstr?;        // name of a variable when used as introspection of a var (optional)
        source_file: cstr?;     // optional source file path
        source_line: i32?;      // optional line number in source

        // kind 
        kind: type_kind;

        // layout / ABI info (useful even at comptime)
        size: usize?;           // sizeof(type) in bytes, null if unsized
        align: usize?;          // alignment in bytes
        stride: usize?;         // stride when used in arrays
        is_pod: bool;           // plain-old-data (no drop / trivial copy)
        is_reference: bool;     // true for 'T*' style non-nullable reference
        pointer_depth: u8;      // 0 = not pointer, >0 pointer indirection count

        // optional: array/slice/tuple specifics
        array_len: isize?;      // >=0 for fixed arrays, -1 for dynamic (slices), null if N/A
        elem_type: typeinfo?;   // element type for arrays/slices/tuples (for tuples this is null)
        tuple_elems: typeinfo[]?; // for tuple types: element types in order

        // runtime hooks exposed for completeness (callable only if generated)
        drop_fn_name: cstr?;    // symbol name or debug hint; null if trivial
        copy_fn_name: cstr?;    // symbol name for copy/clone; null if memcpy
        equal_fn_name: cstr?;   // symbol name for equality; null if memcmp or not provided
        hash_fn_name: cstr?;    // optional hash function symbol name

        // fields / members (for struct-like types)
        struct_fields: field_info[]?; // null if not a struct-like kind

        // enum specifics (for enum-like types)
        enum_variants: variant_info[]?; // null if not an enum

        // function / closure specifics
        is_function: bool;
        is_closure: bool;
        function_args: typeinfo[]?; // arg types in order
        function_return: typeinfo?; // return type (null for void)
        function_varargs: bool;     // true if varargs
        calling_convention: cstr?;  // e.g. "C", "internal", etc.

        // generics and constraints (comptime only)
        generics: generic_param[]?; // names, defaults, constraints
        generic_args: typeinfo[]?;  // concrete generic args if this is an instantiated generic

        // attached/associated functions and methods (comptime metadata)
        attached_functions: method_info[]?;
        inherent_methods: method_info[]?; // methods declared on the type

        // reflection and doc metadata
        attributes: cstr[]?;     // attribute strings like ["inline", "o3"]
        doc: cstr?;              // documentation comment
        visibility: visibility?; // PUBLIC/INTERNAL

        // relationships for analysis tools
        parent_type: typeinfo?;  // if an attached_fn or nested type, reference to owner
        implements_traits: cstr[]?; // trait names (strings) for quick listing

        // flags & helpers
        is_comptime_only: bool; // true (this whole struct is comptime-only)
        is_async: bool;
        is_optional: bool;      // true if T? optional wrapper
        is_array: bool;
        is_slice: bool;
        is_tuple: bool;

        // free-form extension: key/value metadata for future use
        metadata: (cstr, cstr)[]?; // pairs of string keys and values
    }

    // helper sub-structures
    comptime struct field_info { // comptime on a struct means it can only be created/used at comptime
        name: cstr;
        type: typeinfo;          // full nested typeinfo (comptime only)
        offset: usize?;          // offset in bytes if known/applicable
        size: usize?;            // sizeof field if known
        align: usize?;           // alignment of field
        default_expr: cstr?;     // textual default expression (comptime string)
        visibility: visibility?;
        attributes: cstr[]?;
    }

    comptime struct variant_info {
        name: cstr;
        discriminant: i64?;      // explicit discriminant if present
        payload: typeinfo[]?;    // payload types (empty or null for unit variants)
        layout: layout_hint?;    // optional hint about layout/padding for this variant
        attributes: cstr[]?;
    }

    comptime struct generic_param {
        name: cstr;
        param_kind: generic_param_kind;
        default: cstr?;          // textual default if any
        constraints: cstr[]?;    // textual constraints (e.g. "T: trait1 + trait2")
    }

    comptime struct method_info {
        name: cstr;
        signature: typeinfo;     // a typeinfo describing the function/closure signature
        is_static: bool;
        is_attached: bool;       // true if attached to this type
        attributes: cstr[]?;
        visibility: visibility?;
    }

    comptime struct layout_hint {
        // optional advisory layout info used for diagnostics
        size: usize?;
        align: usize?;
        field_offsets: (cstr, usize)[]?; // pairs of field name and offset
    }

    enum type_kind {
        PRIMITIVE,
        POINTER,
        REFERENCE,
        ARRAY,
        SLICE,
        TUPLE,
        STRUCT,
        ENUM,
        FUNCTION,
        CLOSURE,
        TRAIT_OBJECT,
        UNKNOWN
    }

    enum visibility {
        PUBLIC,
        INTERNAL // internal to this project
    }

    enum generic_param_kind {
        TYPE,
        CONST
    }

 *        
 *
 *
 * Keywords:
 *    Above types +
 *    var, val, static, (val is immutable),
 *    intern, // internal to only this project if used (cant be used on struct/enum/error members)
 *    attach, struct, enum, fn, error
 *    comptime, trait, async, true, false,
 *    extern, export,
 *    namespace, use, this, move, copy
 *    if, else, for, while, loop, try, catch, null, match
 *    suspend, resume
 */

<Args: type[]>
extern "C" fn printf(cstr, Args) -> i32;


fn main() -> i32 {
   
    var x = 0; // implicit i32

    val closure = |x*| () { // takes x by reference
         x++; 
    };

    val array: i32[] = 0..100; // exclusive range
    
    for (value, i) in array | value * 2 | { // || gets ran at the start of each iteration
       
       closure();

       std::io::println(value);
       std::io::println(i); // half of value

      /* formatted:
       * std::io::println("Value: {}", value);
       * std::io::println("I: {}", i);
       */
    }

    // we can also do this with errors
    var some_error: error!i32 = 0;
    if (some_error.err) {
        return 1;
    }

    // defer can also be used like in zig:
    // defer some_variable.delete();

    val some_loop_assign = 
        for (value, i) in array | value * 2 | [ var result: i32 ]  // assigns some_loop_assign to result at the last iteration 
    {
       
       closure();

      /* formatted:
       * std::io::println("Value: {}", value);
       * std::io::println("I: {}", i);
       */

       result += x;
    };
    
    :outer for (i) in 0..100 {
        :inner for (j) in 0..=99  {
              if (i == 50) {
                  break: outer;
              }
        }
    }

    while (true) {}

    loop { /* forever until break */ }

    var some_value = some_failing_func() catch |e| {
          return e;
    }; // assigns some_value to e if it fails

    var some_value = try some_failing_func(); // would propagate error
    var some_value = some_failing_func(); // some_value would become error!value
  
    return 0;
}

struct example {
    member: i32;
    member2: f64;
    member3: u8*;
} // Packs as 24 bytes

// But can also be explicit with the "this" param (preferred)
// even though this is passed in, because it is a static function it shouldnt be used unless used like:
// this.member = member;
// this.member2 = member2;
attach fn new(static this: example, member: i32, member2: f64) -> example { 
  return {
        member,
        member2,
        member3: u8::new()
  };
}

attach fn delete(this: example) -> void {
  this.member3.delete(); // Explicitly call delete here, if delete was called on 'this', it will delete all members automatically
}

fn overload_test(x: i32) -> i32 {
    return x;
}

fn overload_test(x: i32, y: i32) -> i32 {
    return x + y;
}

async fn test_async() -> i32 {
    var sum: i32 = 0;
    var i: i32 = 0;
    while (i < 10) {
        sum = sum + i;
        i++;
    }
    return sum;  // Returns 45
}

async fn async_factorial(n: i32) -> i32 {
    if (n <= 1) {
        return 1;
    }
    var result: i32 = 1;
    var i: i32 = 2;
    while (i <= n) {
        result = result * i;
        i++;
    }
    return result;
}

async fn test_suspend_resume() -> i32 {
    var result: i32 = 0;

    // First computation phase
    result = 10;
    suspend;  // Yield control, preserve state

    // Resume here - state preserved
    result = result + 5;  // result is still 10
    suspend;

    // Resume again
    result = result * 2;  // result is now 30
    return result;
}

// Gets ran when a generic uses it and requires all constraints to be true, or an error will occur (compile time)
trait t_allocator  { // naming convention for traits is t_
    <T: type> fn malloc(this, isize) -> T*;
    <T: type> fn realloc(this, T*, isize?) -> T*;
    <T: type> fn free(this, T*) -> void;
} 

// If we want to use the same type for this:
<T: type>
trait t_allocator2 {
    fn malloc(this, isize) -> T*;
    fn realloc(this, T*, isize?) -> T*;
    fn free(this, T*) -> void;
}

namespace std::mem {
    struct default_allocator; // Empty struct, basically a type namespace
    <T: type> // generic arg for the constraint
    attach t_allocator -> default_allocator {
        // Because this is in an attached constraint, we dont need to specify the attach fn, it will do it here
        // Note: because we are in an attached constraint block, we dont need to explicitly set the type of this, as it is known, if we wanted to be explicit though, we could.
        fn malloc(this, size: usize?) -> !T* {
            if (size == null) {
                return @cast<T*>(0); // NOTE: @cast is very dangerous as it can cast from any type to another, use "as T" for a safe cast that allows safe conversions
            } else {
                return @cast<T*>(size);
            }
        }
    
        fn realloc(this, ptr: T*, size: usize) -> !T* {
            return @cast<T*>(size);
        }
    
        fn free(this, ptr: T*) -> void {
            // empty for now
        }
    }
}

// NOTE THESE METHODS WILL BE ATTACHED IN THE STANDARD LIBRARY LIKE THIS:
//
//
<T: type, Allocator: allocator_constraint = std::mem::default_allocator>
attach fn new(static this: T, value: T?, allocator: Allocator?) -> T* {
    var t: T* = allocator.malloc<T>();
    *t = value;
    return t;
} // because this is a template function, it will be auto attached to any type that calls it, such as:

// And corresponding free:
<T: type, Allocator: allocator_constraint = std::default_allocator>
attach fn delete(this: T, allocator: Allocator?) -> void {
    allocator.free<T>(this);
}

// u8::new() or new<u8>()
// With a custom allocator:
// u8::new<my::allocator>() or new<u8, my::allocator>()
//
// This second one works by checking u8 for attached functions and implicitly passing it in as T (as it is the first one used)
//

// Generics can also be inside of structs and enums:

<T: type, C: i32 = 0> // C will default to 0 if theres nothing passed into it
struct some_struct {
  member1: i32 = C; // Default values, allows non initialization of them during construction
  member2: T*?; // implicitly defaults to null
  member3: T*;
}

fn some_fn() -> void {
    var some_struct: some_struct<i32> = { member2: null, member3: &member1 };
    // var some_struct: some_struct<i32>; // default some_struct, will error because member3 cant be defaulted (its a reference)
}

// attaching methods is similar to above, however they must be passed into this

<T: type, C: i32>
attach fn new(static this: some_struct<T, C>) -> somee_struct<T, C> {}

// Enums are similar to rust enums
enum some_reg_enum {
  VALUE,
  OTHER_VALUE,
}

<T: type>
enum generic_enum {
    VALUE: T,
    SOME_OTHER: i32,
    TUPLE_VALUE: (i32, i32),
    NAMED_TUPLE_VALUE: (x: i32, y: i32),
    NO_VALUE
}

// Error enum:
error some_error {
    BLAH,
    BLAHBLAH
}

<T: type>
error some_error2 {
    STRING_MSG: cstr,
    ERROR_TYPE: T
}

// Error function:
fn some_error_thrower() -> some_error!void {
    if (1) { // This will get evaluated at comptime, as its a constexpr
       return some_error::BLAH;
    } else {
       return;
    }
}

fn some_generic_error() -> some_error2<cstr>!void {
    if (1) {
        return some_error2::STRING_MSG("HI");
    } else {
        return error; // generic error
      }
}

fn pointer_array() -> i32 {
    var arr: i32[] = { 5, 10, 15, 20 }; // Array initialization
    var p0: i32* = &(arr[0]);
    var p1: i32* = &(arr[1]);
    var p2: i32* = &(arr[2]);

    return *p0 + *p1 + *p2;  // 5 + 10 + 15 = 30
}

<T: type>
attach fn new(static this: generic_enum<T>) -> void {} // Redundant new


<T: type>
attach fn new(static this: T) -> T {} // Attaches this function to every type, T::new()


<T: type>
attach fn new(static this: T) -> T* {} // Attaches this function to every type, T::new(), overload on return type, requires a context or its ambigious


<T: type, U: type>
attach fn new(static this: T) -> void {} // Overloaded generic function

comptime fn comp() -> type {
    return i32; // type literal can be returned, which allows us to use it in a generic definition:
    /*
        <T: comp()>
        fn some_generic_fn() -> T {}
    */
}

comptime fn comp() -> i32 { // everything in here runs at comptime
    var result = 0; // implicit i32
    for (i) in 0..100 {
        result += i;
    } 
    return result;
}

<C: i32>
fn comp() -> i32 {
    // runs at comptime
    comptime var determined_type: type;
    comptime if (C > 0) {
        determined_type = i32;
    } else {
        determined_type = i8;
    } // this forces all cases to be covered, otherwise error
    // a better option would be to match:
    comptime match (C) {
        C > 0 => determined_type = i32;
        default => { determined_type = i8; }; // match also allows block syntax
    }

    var result: determined_type = 0;
    for (i) in 0..100 {
        result += i;
    } 
    return result;
}

fn optional(some_op: i32?) -> void {
    if (some_op) { // same as calling some_op.value or !some_op.none
        some_op += 1; // no need to do some_op.value (as we know it has one from the above if)
    } else {

    }
}

internal fn some_internal() -> void { } // internal to only this project, (default is public)

@attributes(["inline", "o3", "section:.text"]) // attributes
public comptime async fn sum_range(n: i32) -> i32 {
    var sum: i32 = 0;
    var i: i32 = 0;
    while (i < n) {
        sum = sum + i;
        i++;
    }
    return sum;
}


// Enum values are accessed like:
// var value = generic_enum::NO_VALUE;
// var some_other = generic_enum::SOME_OTHER(1);
// var generic = generic_enum<f32>::VALUE(1.23);

// Builtins:
/*
@typeinfo(type) -> typeinfo
@typeof(type) -> type
@cast<new_type>(variable)
@sizeof(type) -> isize
*/
