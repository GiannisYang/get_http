#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include "def1.h"

#define MAX_HEAP_SIZE 50

template <typename data_type>
class min_heap {
private:
    data_type data[MAX_HEAP_SIZE];
    int heap_size;

public:
    min_heap() {
        memset(data, 0, MAX_HEAP_SIZE);
        heap_size = 0;
    };
    void push(const data_type new_data);
    bool has_elem(const data_type new_data);
    data_type min_elem() { return data[1]; };
    bool empty() { return (heap_size == 0); };
};

/** the declarations and definitions of functions of
  * a template class must be in one header file
  */
template <typename data_type>
void min_heap<data_type>::push(const data_type new_data) {

    int loc, parent;
    if((loc = heap_size + 1) >= MAX_HEAP_SIZE) {
        cout << "min_heap is overflow" << endl;
        return;
    }

    while((parent = loc / 2) > 0) {
        if (data[parent] > new_data) {
            data[loc] = data[parent];
            loc = parent;
        } else { break; }
    }

    data[loc] = new_data;
    ++heap_size;
}

template <typename data_type>
bool min_heap<data_type>::has_elem(const data_type new_data) {
    for(int i = 1; i <= heap_size; i++)
        if(new_data == data[i])
            return true;

    return false;
}

#endif
