#ifndef UNUSED
#ifdef __GNUC__
#define UNUSED(foo) foo __attribute__((unused))
#else
#define UNUSED(foo) foo
#endif
#endif
