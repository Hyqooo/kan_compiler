typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[];
}BufHdr;

void *xrealloc(void *ptr, size_t elem_size) {
    ptr = realloc(ptr, elem_size);
    if (!ptr) {
        perror("xrealloc cannot allocate memory");
        exit(1);
    }
    return ptr;
}

void *xmalloc(size_t num_bytes) {
    void *ptr = malloc(num_bytes);
    if (!ptr) {
        perror("xmalloc cannot allocate memory");
        exit(1);
    }
    return ptr;
}

#define MAX(a, b) (a > b ? a : b)

#define buf__hdr(b) ((BufHdr *)((char *)b - offsetof(BufHdr, buf)))
#define buf__fits(b, n) (buf_len(b) + (n) <= buf_cap(b))
#define buf__fit(b, n) (buf__fits(b, n) ? 0 : ((b) = buf__grow((b), buf_len(b) + (n), sizeof(*(b)))))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_push(b, x) (buf__fit(b, 1), b[buf_len(b)] = (x), buf__hdr(b)->len++)
#define buf_free(b) ((b) ? free(buf__hdr(b)) : 0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size) {
    size_t new_cap = MAX(1 + 2 * buf_cap(buf), new_len);
    assert(new_len <= new_cap);
    size_t new_size = offsetof(BufHdr, buf) + new_cap * elem_size;
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = xrealloc(buf__hdr(buf), new_size);
    } else {
        new_hdr = xmalloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

void buf_test() {
    int *asdf = NULL;
    enum { N = 1024 };
    for (int i = 0; i < N; i++) {
        buf_push(asdf, i);
    }
    assert(buf_len(asdf) == N);
    for (int i = 0; i < buf_len(asdf); i++) {
        assert(asdf[i] == i);
    }
    buf_free(asdf);
}

// Arena allocator
#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_UP(n, a) 


typedef struct Arena {
    char *ptr;
    char *end;
    char **blocks;
}Arena;

#define ARENA_ALIGNMENT 8
#define ARENA_BLOCK_SIZE 1024

void arena_grow(Arena *arena, size_t min_size) {
    size_t size = ALIGN_UP(MAX(ARENA_BLOCK_SIZE, min_size), ARENA_ALIGNMENT);
}


// String interning
typedef struct InternStr {
    size_t len;
    const char *str;
} InternStr;

static InternStr *interns;

const char *str_intern_range(const char *start, const char *end) {
    size_t len = end - start;
    for (size_t i = 0; i < buf_len(interns); i++) {
        if (interns[i].len == len && strncmp(interns[i].str, start, len) == 0) {
            return interns[i].str;
        }
    }
    char *str = xmalloc(len + 1);
    memcpy(str, start, len);
    str[len] = 0;
    buf_push(interns, ((InternStr){len, str}));
    return str;
}

const char *str_intern(const char *str) {
    return str_intern_range(str, str + strlen(str));
}

void str_intern_test() {
    char x[] = "hello";
    char y[] = "hello";
    assert(x != y);
    const char *px = str_intern(x);
    const char *py = str_intern(y);
    assert(px == py);
    char z[] = "hello!";
    const char *pz = str_intern(z);
    assert(pz != px);
}