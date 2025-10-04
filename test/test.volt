use std::io;

// import C headers into a C namespace
use { test.h, test2.h } as c;
use { test.hpp, test2.hpp } as cpp;

// c::some_fn();
// cpp::some_temlpate<i32>(1);
//

/*
 * Types:
 *    i8, i16, i32, i64, i128
 *    u8, u16, u32, u64, u128
 *        f16, f32, f64, f128
 *    bool
 *    isize, usize
 *    cstr <-- null terminated, str 
 *
 *    T[..] // Slice
 *    T[]   // Array
 *    T*    // pointer
 *    T&    // Reference
 *    T?    // Optional
 *    (T, ...) // tuple
 *
 *    typeinfo: // basically a struct of:
 *        name: cstr
 *        module: cstr
 *        namespace: cstr
 *        function: bool
 *        struct: bool
 *        enum: bool
 *        generics: type[]?
 *        attached_fn: typeinfo? <-- parent
 *        parent: typeinfo? <-- parent struct
 *        comptime: bool
 *        async: bool
 *
 *
 * Keywords:
 *    Above types +
 *    const, var, let, static,
 *    attach, struct, enum, fn, error
 *    comptime, constraint, async, true, false,
 *    extern, export,
 *    namespace, use, this, move, copy
 *    if, else, for, while, loop, try, catch, null
 *    suspend, resume
 */

<Args: type[]>
extern C // Compiler interprets this
fn printf(cstr, Args);

fn main() -> i32 {
   
    var x = 0; // implicit i32

    let closure = |&x| () {
         x++; 
    };

    let array: i32[] = 0..100; // exclusive range
    
    for (value, i) in array | value * 2 | { 
       
       closure();

       std::io::println(value);
       std::io::println(i); // half of value

      /* formatted:
       * std::io::println("Value: {}", value);
       * std::io::println("I: {}", i);
       */
    }
    
    for (i) in 0..100 :outer {
        for (j) in 0..=99 :inner {
              if (i == 50) {
                  break: outer;
              }
        }
    }

    while (/* some_condition */) {}

    loop { /* forever until break */ }

    var some_value = some_failing_func() catch |e| {
          e
      }; // assigns some_value to e if it fails

    // var some_value = try some_failing_func(); // would propagate error
    // var some_value = some_failing_func(); // some_value would become error!value
  
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
  this.member3.delete();
}

// Gets ran when a generic uses it and requires all constraints to be true, or an error will occur (compile time)
constraint alllocator_constraint {
/* Just a label ->*/ malloc:  has fn<T: type>(isize) -> T*;
/* Just a label ->*/ realloc: has fn<T: type>(T*, isize?) -> T*;
/* Just a label ->*/ free:    has fn<T: type>(T*) -> void;
}

// CONSTRAINTS
// <T: type>
// constraint some_constraint {
//    of_type: is T*
//    has_fn:  has fn(i32) -> void;
// }

// NOTE THESE METHODS WILL BE ATTACHED IN THE STANDARD LIBRARY LIKE THIS:
//
//
// <T: type, Allocator: allocator_constraint = std::default_allocator>
// attach fn new(static this: T, value: T?, allocator: Allocator?) -> T* {
//    var t: T* = allocator::malloc<T>();
//    *t = value;
//    // could also deref like this (like zig) when chaining:
//    // t.* = value;
//    return t;
// } // because this is a template function, it will be auto attached to any type that calls it, such as:
//
// // And corresponding free:
// <T: type, Allocator: allocator_constraint = std::default_allocator>
// attach fn delete(this: T, allocator: Allocator?) -> void {
//    allocator::free<T>(this);
// }
//
// u8::new() or new<u8>()
// With a custom allocator:
// u8::new<my::allocator>() or new<u8, my::allocator>()
//
// This second one works by checking u8 for attached functions and implicitly passing it in as T (as it is the first one used)
//

// Generics can also be inside of structs and enums:

<T: type, C: i32>
struct some_struct {
  member1: T;
  member2: T*;
  member3: T&;
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
fn some_error_thrower() some_error!void {
    if (1) { // This will get evaluated at comptime, as its a constexpr
       return some_error::BLAH;
    } else {
       return;
    }
}

fn some_generic_error() some_error2<cstr>!void {
    if (1) {
        return some_error2::STRING_MSG("HI");
    } else {
        return error; // generic error
      }
}

<T: type>
attach fn new(static this: generic_enum<T>) -> void {} // Redundant new

// Enum values are accessed like:
// var value = generic_enum::NO_VALUE;
// var some_other = generic_enum::SOME_OTHER(1);
// var generic = generic_enum<f32>::VALUE(1.23);

// Builtins:
@typeof(type) -> typeinfo
@cast<new_type>(variable)
@sizeof(type) -> isize
