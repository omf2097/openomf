#ifndef _BK_H
#define _BK_H

typedef struct bk_file_t {

} bk_file;

bk_file* bk_parse(const char* data, int len);
void bk_destroy(bk_file* bkfile);

#endif // _BK_H