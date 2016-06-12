#include <stdlib.h>
#include "stack.h"

t_stack *stack_create() {
	t_stack* stack = malloc(sizeof(t_stack));
	t_list* elements = list_create();
	stack->elements = elements;
	return stack;
}

void stack_clean(t_stack *self) {
	list_clean(self->elements);
}

void stack_clean_and_destroy_elements(t_stack *self, void(*element_destroyer)(void*)) {
	list_clean_and_destroy_elements(self->elements, element_destroyer);
}

void stack_destroy(t_stack *self) {
	list_destroy(self->elements);
	free(self);
}

void stack_destroy_and_destroy_elements(t_stack *self, void(*element_destroyer)(void*)) {
	list_destroy_and_destroy_elements(self->elements, element_destroyer);
	free(self);
}

// Override de push:
void stack_push(t_stack *self, void *element) {
	list_add_in_index(self->elements, 0, element);
}

// Override de pop:
void *stack_pop(t_stack *self) {
	int index = NELEMS(self->elements) - 1;
	return list_remove(self->elements, index);
}

// Override de peek:
void *stack_peek(t_stack *self) {
	int index = NELEMS(self->elements) - 1;
	return list_get(self->elements, index);
}

int stack_size(t_stack* self) {
	return list_size(self->elements);
}

int stack_is_empty(t_stack *self) {
	return list_is_empty(self->elements);
}
