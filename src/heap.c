#include "../include/codexion.h"

void heap_init(t_heap *heap, int capacity)
{
    heap->data = malloc(sizeof(t_request) * capacity);
    heap->size = 0;
    heap->capacity = capacity;
}

static int compare(t_request a, t_request b)
{
    if (a.priority_key < b.priority_key) return (-1);
    if (a.priority_key > b.priority_key) return (1);
    // Tie-breaker rule (EDF/FIFO determinismo)
    if (a.sequence_num < b.sequence_num) return (-1);
    return (1);
}

void heap_insert(t_heap *heap, t_request req)
{
    int i = heap->size++;
    heap->data[i] = req;
    
    while (i > 0)
    {
        int p = (i - 1) / 2;
        if (compare(heap->data[i], heap->data[p]) >= 0) break;
        t_request tmp = heap->data[i];
        heap->data[i] = heap->data[p];
        heap->data[p] = tmp;
        i = p;
    }
}

t_request heap_peek(t_heap *heap)
{
    return (heap->data[0]);
}

void heap_extract_min(t_heap *heap)
{
    if (heap->size <= 0) return;
    heap->data[0] = heap->data[--heap->size];
    
    int i = 0;
    while (1)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        
        if (left < heap->size && compare(heap->data[left], heap->data[smallest]) < 0)
            smallest = left;
        if (right < heap->size && compare(heap->data[right], heap->data[smallest]) < 0)
            smallest = right;
        if (smallest == i) break;
        
        t_request tmp = heap->data[i];
        heap->data[i] = heap->data[smallest];
        heap->data[smallest] = tmp;
        i = smallest;
    }
}

void heap_remove_coder(t_heap *heap, int coder_id)
{
    int idx = -1;
    for (int i = 0; i < heap->size; i++)
    {
        if (heap->data[i].coder_id == coder_id)
        {
            idx = i;
            break;
        }
    }
    if (idx == -1) return;
    
    // Substitui pelo último e refaz o heap (simplificado limpando e reinserindo)
    t_heap tmp;
    heap_init(&tmp, heap->capacity);
    for (int i = 0; i < heap->size; i++)
    {
        if (i != idx)
            heap_insert(&tmp, heap->data[i]);
    }
    free(heap->data);
    *heap = tmp;
}
