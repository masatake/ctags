package ffi

foreign import libc "system:libc.so"

foreign libc {
    puts :: proc "c" (s: cstring) -> i32 ---
    exit :: proc "c" (code: i32) ---
    errno: i32
    signal_handler: proc "c" (sig: i32)
}

foreign import mruby "libmruby.a"

@(default_calling_convention = "c")
foreign mruby {
    @(link_name = "mrb_open")
    open :: proc() -> rawptr ---
    close :: proc(state: rawptr) ---
}

foreign import gl { "system:opengl32.lib", "system:glu32.lib" }

LIB :: ""

foreign import stbi {
    LIB when LIB != "" else "system:stb_image",
}

foreign libc {
    @private
    internal_init :: proc "c" () ---
    readv :: proc "c" (fd: i32, iov: rawptr, cnt: i32) -> i32 ---
    getres :: proc "c" (w: ^i32, h: ^i32) -> (i32, i32) ---
    printf :: proc "c" (fmt: cstring, #c_vararg args: ..any) -> i32 ---
    get_buf :: proc "c" (n: i32) -> [4]i32 ---
    when ODIN_OS == .Linux {
        epoll_create :: proc "c" (size: i32) -> i32 ---
    }
}

// Taken from https://odin-lang.org/docs/overview/#foreign-system
foreign import lowlevel "lowlevel.asm"
foreign lowlevel {
    __get_flags :: proc "c" () -> u64 ---
}

foreign import lowlevelalt {
    "lowlevelalt0.s",
    "lowlevelalt0.S"
}

/* The collection is an empty string. The parser should not crash. */
foreign import broken0 ":broken0.so"
foreign import broken1 ":broken1.s"

/* The ccode (or asmfile) is an empty string. The parser should not crash. */
foreign import broken2 "system:"
