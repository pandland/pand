#include <criterion/criterion.h>
#include <unistd.h>
#include <stdio.h>

#include "../src/heap.h"

struct ttimer {
  int timeout;
  struct heap_node hnode;
} typedef ttimer_t;

int timer_comparator(struct heap_node *a, struct heap_node *b) {
  ttimer_t *child = container_of(a, ttimer_t, hnode);
  ttimer_t *parent = container_of(b, ttimer_t, hnode);
  //printf("%d vs %d\n", parent->timeout, child->timeout);
  if (child->timeout < parent->timeout) {
    return 1;
  }

  return 0;
}

ttimer_t init_timer(int timeout) {
  ttimer_t timer;
  timer.timeout = timeout;
  heap_init_node(&timer.hnode);

  return timer;
}

void print_heap_until(struct heap_node *node, int space) {
    if (node == NULL) {
      return;
    }
    int count = 20;
    space += count;
    print_heap_until(node->right, space);
    printf("\n");
    for (int i = count; i < space; i++)
        printf(" ");
    ttimer_t *timer = container_of(node, ttimer_t, hnode);
    printf("%d\n", timer->timeout);
    print_heap_until(node->left, space);
}

void print_heap(struct heap *h) {
  if (h->root == NULL) {
    return;
  }
  print_heap_until(h->root, 0);
}

Test(heap, problem_of_4) {
  struct heap h;
  heap_init(&h, timer_comparator);
  ttimer_t timer1 = init_timer(200);
  ttimer_t timer2 = init_timer(400);
  ttimer_t timer3 = init_timer(500);
  ttimer_t timer4 = init_timer(600);

  heap_insert(&h, &timer1.hnode);
  heap_insert(&h, &timer2.hnode);
  heap_insert(&h, &timer3.hnode);
  heap_insert(&h, &timer4.hnode);

  heap_remove(&h, &timer1.hnode);
  //print_heap(&h);
  heap_remove(&h, &timer2.hnode);
  //print_heap(&h);
  heap_remove(&h, &timer3.hnode);
  //print_heap(&h);
}

Test(heap, remove) {
  struct heap h;
  heap_init(&h, timer_comparator);
  
  ttimer_t timer1 = init_timer(200);
  ttimer_t timer2 = init_timer(10);
  ttimer_t timer3 = init_timer(60);
  ttimer_t timer4 = init_timer(5);
  ttimer_t timer5 = init_timer(2);
  ttimer_t timer6 = init_timer(40);
  ttimer_t timer7 = init_timer(2200);
  ttimer_t timer8 = init_timer(33);
  ttimer_t timer9 = init_timer(6);
  ttimer_t timer10 = init_timer(232);
  ttimer_t timer11 = init_timer(241);
  ttimer_t timer12 = init_timer(133);

  heap_insert(&h, &timer1.hnode);
  heap_insert(&h, &timer2.hnode);
  heap_insert(&h, &timer3.hnode);
  heap_insert(&h, &timer4.hnode);
  heap_insert(&h, &timer5.hnode);
  heap_insert(&h, &timer6.hnode);
  heap_insert(&h, &timer7.hnode);
  heap_insert(&h, &timer8.hnode);
  heap_insert(&h, &timer9.hnode);
  heap_insert(&h, &timer10.hnode);
  heap_insert(&h, &timer11.hnode);
  heap_insert(&h, &timer12.hnode);
  //print_heap(&h);
  //printf("============================================================\n");
  struct heap_node *min_node = heap_pop(&h);
  ttimer_t *min_timer = container_of(min_node, ttimer_t, hnode);
  cr_expect(min_timer->timeout == 2, "Expected 2, got %d", min_timer->timeout);
  //printf("============================================================\n");
  //print_heap(&h);
  min_node = heap_pop(&h);
  min_timer = container_of(min_node, ttimer_t, hnode);
  cr_expect(min_timer->timeout == 5, "Expected 5, got %d", min_timer->timeout);
  //printf("============================================================\n");
  //print_heap(&h);
  heap_remove(&h, &timer6.hnode);
  struct heap_node *mid_node = heap_remove(&h, &timer8.hnode);
  ttimer_t *mid_timer = container_of(mid_node, ttimer_t, hnode);
  cr_expect(mid_timer->timeout == timer8.timeout, "Expected %d, got %d", timer8.timeout, mid_timer->timeout);

  //printf("============================================================\n");
  print_heap(&h);
}
