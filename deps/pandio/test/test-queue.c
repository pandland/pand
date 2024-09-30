#include <criterion/criterion.h>
#include "../src/queue.h"

typedef struct test_task {
  int id;
  struct queue_node qnode;
} test_task_t;

#define TEST_TASKS 100

Test(queue, test) {
  struct queue tasks;
  queue_init(&tasks);
  
  for (int i = 0; i < TEST_TASKS; ++i) {
    test_task_t *task = malloc(sizeof(test_task_t));
    task->id = i;
    queue_init_node(&task->qnode);
    queue_push(&tasks, &task->qnode);
  }
  
  cr_expect_neq(tasks.head, NULL);
  cr_expect_neq(tasks.tail, NULL);
  cr_expect_neq(tasks.head, tasks.tail);

  for (int i = 0; i < TEST_TASKS; ++i) {
    struct queue_node *task_node = queue_pop(&tasks);
    test_task_t *task = container_of(task_node, test_task_t, qnode);
    cr_expect_eq(task->id, i);
    free(task);
  }

  cr_expect_eq(tasks.head, NULL);
  cr_expect_eq(tasks.tail, NULL);
}
