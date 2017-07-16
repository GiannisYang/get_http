#ifndef TOOLS_H
#define TOOLS_H

#include "def1.h"

bool has_str(const u_char* p, const char *str);
u_char *get_loc_ip();
int get_num(const u_char* p, const char terminator);

int z_compress(Bytef *data, uLong ndata,
Bytef *zdata, uLong *nzdata);
int gz_compress(Bytef *data, uLong ndata,
Bytef *zdata, uLong *nzdata);
int z_decompress(Byte *zdata, uLong nzdata,
Byte *data, uLong *ndata);
int http_gz_decompress(Byte *zdata, uLong nzdata,
Byte *data, uLong *ndata);
int gz_decompress(Byte *zdata, uLong nzdata,
Byte *data, uLong *ndata);

#endif
