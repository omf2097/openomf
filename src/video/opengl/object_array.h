#ifndef OBJECT_ARRAY_H
#define OBJECT_ARRAY_H

typedef struct object_array object_array;

object_array *object_array_create();
void object_array_free(object_array **array);

void object_array_prepare(object_array *array);
void object_array_finish(object_array *array);
void object_array_draw(const object_array *array);
void object_array_add(object_array *array, int x, int y, int w, int h);

#endif // OBJECT_ARRAY_H
