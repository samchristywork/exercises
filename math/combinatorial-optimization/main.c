#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Item {
  int weight;
  int value;
} Item;

typedef struct KnapsackSolution {
  Item *selected;
  int n;
  int value;
  int remaining_capacity;
} KnapsackSolution;

typedef struct SubsetSumSolution {
  int *selected;
  int n;
  int value;
  int remaining_capacity;
} SubsetSumSolution;

KnapsackSolution knapsack(Item items[], int n, int capacity) {
  if (n == 0 || capacity == 0) {
    return (KnapsackSolution){
      .selected = NULL,
      .n = 0,
      .value = 0,
      .remaining_capacity = capacity
    };
  }

  if (n==1) {
    Item *selected_items = (Item *)malloc(1 * sizeof(Item));

    if (items[0].weight <= capacity) {
      selected_items[0] = items[0];
    } else {
      selected_items[0].weight = 0;
      selected_items[0].value = 0;
    }

    return (KnapsackSolution){
      .selected = selected_items,
      .n = 1,
      .value = selected_items[0].value,
      .remaining_capacity = capacity - selected_items[0].weight
    };
  }

  // Build the table
  int dp[n + 1][capacity + 1];
  for (int i = 0; i <= n; i++) {
    for (int w = 0; w <= capacity; w++) {
      if (i == 0 || w == 0) {
        dp[i][w] = 0;
      } else if (items[i - 1].weight <= w) {
        // Consider the item or do not consider it, take the maximum
        dp[i][w] =
            (items[i - 1].value + dp[i - 1][w - items[i - 1].weight] >
             dp[i - 1][w])
                ? (items[i - 1].value + dp[i - 1][w - items[i - 1].weight])
                : (dp[i - 1][w]);
      } else {
        // Do not take the item
        dp[i][w] = dp[i - 1][w];
      }
    }
  }

  // The last cell of the table will have the answer
  int result = dp[n][capacity];

  Item *selected_items = (Item *)malloc(n * sizeof(Item));
  int count = 0;
  int w = capacity;

  for (int i = n; i > 0 && result > 0; i--) {
    if (result != dp[i - 1][w]) {
      // This item is included.
      selected_items[count++] = items[i - 1];
      // We remove this item's weight from the total weight
      w -= items[i - 1].weight;
      // Subtract the item's value from the result to continue tracing back
      result -= items[i - 1].value;
    }
  }

  return (KnapsackSolution){
    .selected = selected_items,
    .n = count,
    .value = dp[n][capacity],
    .remaining_capacity = w
  };
}

void printKnapsackSolution(KnapsackSolution solution) {
  printf("Maximum value in knapsack = %d\n", solution.value);
  printf("Remaining capacity = %d\n", solution.remaining_capacity);
  printf("Number of items included = %d\n", solution.n);
  printf("Items included in the knapsack:\n");
  for (int i = 0; i < solution.n; i++) {
    printf("Item %d: Weight = %d, Value = %d\n", i + 1, solution.selected[i].weight,
           solution.selected[i].value);
  }
}

SubsetSumSolution subsetSum(int set[], int n, int capacity) {
  Item items[n];
  for (int i = 0; i < n; i++) {
    items[i].weight = set[i];
    items[i].value = set[i];
  }

  KnapsackSolution knapsack_solution = knapsack(items, n, capacity);

  int *selected = (int *)malloc(knapsack_solution.n * sizeof(int));
  for (int i = 0; i < knapsack_solution.n; i++) {
    selected[i] = knapsack_solution.selected[i].weight;
  }

  free(knapsack_solution.selected);

  return (SubsetSumSolution){
    .selected = selected,
    .n = knapsack_solution.n,
    .value = knapsack_solution.value,
    .remaining_capacity = knapsack_solution.remaining_capacity
  };
}

void printSubsetSumSolution(SubsetSumSolution solution) {
  printf("Maximum value in subset sum = %d\n", solution.value);
  printf("Remaining capacity = %d\n", solution.remaining_capacity);
  printf("Number of items included = %d\n", solution.n);
  printf("Items included in the subset sum:\n");
  for (int i = 0; i < solution.n; i++) {
    printf("Item %d: Weight = %d\n", i + 1, solution.selected[i]);
  }
}

#ifdef TEST
bool testEqual(int actual, int expected) {
  return actual == expected;
}

int compareItems(const void *a, const void *b) {
  Item *itemA = (Item *)a;
  Item *itemB = (Item *)b;
  if (itemA->weight != itemB->weight) {
    return itemA->weight - itemB->weight;
  }
  return itemA->value - itemB->value;
}

int compareInts(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}

bool testItemArrayEqual(Item *actual, Item *expected, int size) {
  qsort(actual, size, sizeof(Item), compareItems);
  qsort(expected, size, sizeof(Item), compareItems);

  for (int i = 0; i < size; i++) {
    if (actual[i].weight != expected[i].weight ||
        actual[i].value != expected[i].value) {
      return false;
    }
  }

  return true;
}

bool testIntArrayEqual(int *actual, int *expected, int size) {
  qsort(actual, size, sizeof(int), compareInts);
  qsort(expected, size, sizeof(int), compareInts);

  for (int i = 0; i < size; i++) {
    if (actual[i] != expected[i]) {
      return false;
    }
  }

  return true;
}

bool testSubsetSumSolutionEqual(SubsetSumSolution actual, SubsetSumSolution expected) {
  if (!testEqual(actual.value, expected.value) ||
      !testEqual(actual.remaining_capacity, expected.remaining_capacity) ||
      !testEqual(actual.n, expected.n)) {
    return false;
  }
  return testIntArrayEqual(actual.selected, expected.selected, actual.n);
}

void checkSubsetSumSolution(SubsetSumSolution actual, SubsetSumSolution expected) {
  if (!testSubsetSumSolutionEqual(actual, expected)) {
    printf("Test failed!\n");
    printf("\n");
    printf("Actual:\n");
    printSubsetSumSolution(actual);
    printf("\n");
    printf("Expected:\n");
    printSubsetSumSolution(expected);

    exit(1);
  }
}

int main() {
  // Test case 1
  {
    int capacity = 50;
    Item items[] = {{10, 60}, {20, 100}, {30, 120}};
    int n = sizeof(items) / sizeof(items[0]);

    Item expected_items[] = {{20, 100}, {30, 120}};
    int expected_max_value = 220;
    int expected_remaining_capacity = 0;
    int expected_n_items = 2;

    int max_value;
    int remaining_capacity;
    int n_items;
    Item *selection = knapsack(items, n, capacity, &max_value,
                               &remaining_capacity, &n_items);

    checkTest("Test Case 1", selection, n_items, expected_items,
                   expected_n_items, max_value, expected_max_value,
                   remaining_capacity, expected_remaining_capacity);

    free(selection);
  }

  printf("All test cases passed!\n");
}

#else
int main() {
  int capacity = 50;
  Item items[] = {{10, 60}, {20, 100}, {20, 30}};
  int n = sizeof(items) / sizeof(items[0]);

  KnapsackSolution solution = knapsack(items, n, capacity);
  printKnapsackSolution(solution);
  free(solution.selected);
}
#endif
