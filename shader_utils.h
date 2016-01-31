// Returns -1 on error. Prints error to stdout.
// Kind is GL_FRAGMENT_SHADER, GL_VERTEX_SHADER, ...
i32 make_shader(u32 kind, char * source, u32 length);
// Returns -1 on error.
i32 load_simple_shader(char * vertsrc, i32 vslen, char * fragsrc, i32 fslen);
// Returns -1 on error.
i32 load_simple_shader2(char * geomsrc, i32 gslen, char * vertsrc, i32 vslen, char * fragsrc, i32 fslen);
