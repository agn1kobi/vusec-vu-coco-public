#include <stdio.h>
#include <stdlib.h>

void __coco_dummy_print_allocation(int elems) {
    printf("Allocating %d elements on stack\n", elems);
}

void __coco_check_bounds(int offset, int array_size) {
    if (offset < 0 || offset >= array_size) {
        fprintf(stderr, "Error: Array index out of bounds. Offset: %d, Array size: %d\n", offset, array_size);
        exit(-1);
    }
}