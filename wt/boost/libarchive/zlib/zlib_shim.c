/* zlib_shim.c
   Forward plain zlib API names to z_*-prefixed symbols present in zs.lib.
   Compile with MSVC and pack into zlib_shim.lib, link shim before zs.lib.
*/

#ifdef _MSC_VER
#define ZLIBAPI __cdecl
#else
#define ZLIBAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Minimal z_stream so signatures match. If you have <zlib.h> available,
   you may prefer to include it and remove this typedef. */
typedef struct z_stream_s {
    unsigned char *next_in; unsigned int avail_in;
    unsigned long total_in;
    unsigned char *next_out; unsigned int avail_out;
    unsigned long total_out;
    void *state;
    void *zalloc; void *zfree; void *opaque;
    int data_type; unsigned long adler; unsigned long reserved;
} z_stream;
typedef z_stream *z_streamp;

/* Forward declarations of the z_ prefixed symbols expected to exist in zs.lib */
int ZLIBAPI z_deflate(z_streamp strm, int flush);
int ZLIBAPI z_deflateEnd(z_streamp strm);
int ZLIBAPI z_deflateInit_(z_streamp strm, int level, const char *version, int stream_size);
int ZLIBAPI z_deflateInit2_(z_streamp strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size);
unsigned long ZLIBAPI z_crc32(unsigned long crc, const unsigned char *buf, unsigned int len);
unsigned long ZLIBAPI z_adler32(unsigned long adler, const unsigned char *buf, unsigned int len);
int ZLIBAPI z_inflate(z_streamp strm, int flush);
int ZLIBAPI z_inflateEnd(z_streamp strm);
int ZLIBAPI z_inflateInit_(z_streamp strm, const char *version, int stream_size);
int ZLIBAPI z_inflateInit2_(z_streamp strm, int windowBits, const char *version, int stream_size);
int ZLIBAPI z_compress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);
int ZLIBAPI z_uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);
int ZLIBAPI z_compress2(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen, int level);

/* Provide the plain (non-prefixed) API expected by consumers like libharu */
int ZLIBAPI deflate(z_streamp strm, int flush) { return z_deflate(strm, flush); }
int ZLIBAPI deflateEnd(z_streamp strm) { return z_deflateEnd(strm); }
int ZLIBAPI deflateInit_(z_streamp strm, int level, const char *version, int stream_size) { return z_deflateInit_(strm, level, version, stream_size); }
int ZLIBAPI deflateInit2_(z_streamp strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size) { return z_deflateInit2_(strm, level, method, windowBits, memLevel, strategy, version, stream_size); }
unsigned long ZLIBAPI crc32(unsigned long crc, const unsigned char *buf, unsigned int len) { return z_crc32(crc, buf, len); }
unsigned long ZLIBAPI adler32(unsigned long adler, const unsigned char *buf, unsigned int len) { return z_adler32(adler, buf, len); }
int ZLIBAPI inflate(z_streamp strm, int flush) { return z_inflate(strm, flush); }
int ZLIBAPI inflateEnd(z_streamp strm) { return z_inflateEnd(strm); }
int ZLIBAPI inflateInit_(z_streamp strm, const char *version, int stream_size) { return z_inflateInit_(strm, version, stream_size); }
int ZLIBAPI inflateInit2_(z_streamp strm, int windowBits, const char *version, int stream_size) { return z_inflateInit2_(strm, windowBits, version, stream_size); }
int ZLIBAPI compress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen) { return z_compress(dest, destLen, source, sourceLen); }
int ZLIBAPI uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen) { return z_uncompress(dest, destLen, source, sourceLen); }
int ZLIBAPI compress2(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen, int level) { return z_compress2(dest, destLen, source, sourceLen, level); }

#ifdef __cplusplus
}
#endif
