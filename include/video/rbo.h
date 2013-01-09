#ifndef _RBO_H
#define _RBO_H

typedef struct rbo_t {
    unsigned int id;
    unsigned int w,h;
} rbo;

void rbo_create(rbo *rbo, unsigned int w, unsigned int h);
void rbo_free(rbo *rbo);
void rbo_bind(rbo *rbo);
void rbo_unbind();

#endif // _RBO_H