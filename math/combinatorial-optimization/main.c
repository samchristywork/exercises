#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
        .selected = NULL, .n = 0, .value = 0, .remaining_capacity = capacity};
  }

  if (n == 1) {
    Item *selected_items = (Item *)malloc(1 * sizeof(Item));
    Item *selected_item = &selected_items[0];
    Item item = items[0];

    if (item.weight <= capacity) {
      selected_items[0] = item;
    } else {
      selected_item->weight = 0;
      selected_item->value = 0;
    }

    return (KnapsackSolution){.selected = selected_items,
                              .n = 1,
                              .value = selected_item->value,
                              .remaining_capacity =
                                  capacity - selected_item->weight};
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

  return (KnapsackSolution){.selected = selected_items,
                            .n = count,
                            .value = dp[n][capacity],
                            .remaining_capacity = w};
}

void printKnapsackSolution(KnapsackSolution solution) {
  printf("Maximum value in knapsack = %d\n", solution.value);
  printf("Remaining capacity = %d\n", solution.remaining_capacity);
  printf("Number of items included = %d\n", solution.n);
  printf("Items included in the knapsack:\n");
  for (int i = 0; i < solution.n; i++) {
    printf("Item %d: Weight = %d, Value = %d\n", i + 1,
           solution.selected[i].weight, solution.selected[i].value);
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

  return (SubsetSumSolution){.selected = selected,
                             .n = knapsack_solution.n,
                             .value = knapsack_solution.value,
                             .remaining_capacity =
                                 knapsack_solution.remaining_capacity};
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
bool testEqual(int actual, int expected) { return actual == expected; }

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

bool testKnapsackSolutionEqual(KnapsackSolution actual,
                               KnapsackSolution expected) {
  if (!testEqual(actual.value, expected.value) ||
      !testEqual(actual.remaining_capacity, expected.remaining_capacity) ||
      !testEqual(actual.n, expected.n)) {
    return false;
  }

  return testItemArrayEqual(actual.selected, expected.selected, actual.n);
}

bool testSubsetSumSolutionEqual(SubsetSumSolution actual,
                                SubsetSumSolution expected) {
  if (!testEqual(actual.value, expected.value) ||
      !testEqual(actual.remaining_capacity, expected.remaining_capacity) ||
      !testEqual(actual.n, expected.n)) {
    return false;
  }
  return testIntArrayEqual(actual.selected, expected.selected, actual.n);
}

void checkKnapsackSolution(KnapsackSolution actual, KnapsackSolution expected) {
  if (!testKnapsackSolutionEqual(actual, expected)) {
    printf("Test failed!\n");
    printf("\n");
    printf("Actual:\n");
    printKnapsackSolution(actual);
    printf("\n");
    printf("Expected:\n");
    printKnapsackSolution(expected);

    exit(1);
  }
}

void checkSubsetSumSolution(SubsetSumSolution actual,
                            SubsetSumSolution expected) {
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
    Item items[] = {{10, 60}, {20, 100}, {30, 30}};
    int n = sizeof(items) / sizeof(items[0]);

    KnapsackSolution expected = {.selected = (Item[]){items[0], items[1]},
                                 .n = 2,
                                 .value = 160,
                                 .remaining_capacity = 20};

    KnapsackSolution solution = knapsack(items, n, capacity);
    checkKnapsackSolution(solution, expected);

    free(solution.selected);
  }

  // Test case 2
  {
    int capacity = 0;
    Item items[] = {{10, 60}, {20, 100}, {30, 30}};
    int n = sizeof(items) / sizeof(items[0]);

    KnapsackSolution expected = {
        .selected = (Item[]){}, .n = 0, .value = 0, .remaining_capacity = 0};

    KnapsackSolution solution = knapsack(items, n, capacity);
    checkKnapsackSolution(solution, expected);

    free(solution.selected);
  }

  // Test case 3
  {
    int capacity = 50;
    Item items[] = {};
    int n = sizeof(items) / sizeof(items[0]);

    KnapsackSolution expected = {
        .selected = (Item[]){}, .n = 0, .value = 0, .remaining_capacity = 50};

    KnapsackSolution solution = knapsack(items, n, capacity);
    checkKnapsackSolution(solution, expected);

    free(solution.selected);
  }

  // Test case 4
  {
    int capacity = 50;
    Item items[] = {{60, 100}, {100, 200}, {120, 300}};
    int n = sizeof(items) / sizeof(items[0]);

    KnapsackSolution expected = {
        .selected = (Item[]){}, .n = 0, .value = 0, .remaining_capacity = 50};

    KnapsackSolution solution = knapsack(items, n, capacity);
    checkKnapsackSolution(solution, expected);

    free(solution.selected);
  }

  // Test case 5
  {
    int capacity = 50;
    Item items[] = {{10, 60}, {20, 100}, {30, 120}};
    int n = sizeof(items) / sizeof(items[0]);

    KnapsackSolution expected = {.selected = (Item[]){items[1], items[2]},
                                 .n = 2,
                                 .value = 220,
                                 .remaining_capacity = 0};

    KnapsackSolution solution = knapsack(items, n, capacity);
    checkKnapsackSolution(solution, expected);

    free(solution.selected);
  }

  // Test case 6
  {
    int capacity = 9;
    int set[] = {1, 2, 3, 4, 5};
    int n = sizeof(set) / sizeof(set[0]);

    SubsetSumSolution expected = {.selected = (int[]){set[1], set[2], set[3]},
                                  .n = 3,
                                  .value = 9,
                                  .remaining_capacity = 0};

    SubsetSumSolution solution = subsetSum(set, n, capacity);
    checkSubsetSumSolution(solution, expected);

    free(solution.selected);
  }

  printf("All test cases passed!\n");
}

#else
void usage(char *name) {
  printf("Usage: %s [options]\n", name);
  printf("\n");
  printf("Options:\n");
  printf("  -h, --help                 Show this help message\n");
  printf("  -c, --capacity <capacity>  Set the capacity of the knapsack\n");
  printf("  -s, --subset-sum           Solve the subset sum problem\n");
  printf("\n");
}

int main(int argc, char *argv[]) {
  int capacity = 0;
  bool subsetSumFlag = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      usage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "-c") == 0 ||
               strcmp(argv[i], "--capacity") == 0) {
      if (i + 1 < argc) {
        capacity = atoi(argv[++i]);
      } else {
        fprintf(stderr, "Error: Missing capacity value\n");
        return 1;
      }
    } else if (strcmp(argv[i], "-s") == 0 ||
               strcmp(argv[i], "--subset-sum") == 0) {
      subsetSumFlag = true;
    } else if (strcmp(argv[i], "-k") == 0 ||
               strcmp(argv[i], "--knapsack") == 0) {
      subsetSumFlag = false;
    } else {
      fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
      return 1;
    }
  }

  if (subsetSumFlag) {
    int n = 0;
    int items_size = 100;
    int *items = malloc(items_size * sizeof(int));

    while (true) {
      int item;
      int r = scanf("%d", &item);
      if (r == EOF) {
        break;
      }

      if (r != 1) {
        fprintf(stderr, "Invalid input format\n");
        free(items);
        return 1;
      }

      if (n >= items_size) {
        items_size *= 2;
        items = realloc(items, items_size * sizeof(int));
      }

      items[n++] = item;
    }

    SubsetSumSolution solution = subsetSum(items, n, capacity);
    printSubsetSumSolution(solution);

    free(solution.selected);
  } else {
    int n = 0;

    int items_size = 100;
    Item *items = malloc(items_size * sizeof(Item));

    while (true) {
      Item item;
      int r = scanf("%d %d", &item.weight, &item.value);
      if (r == EOF) {
        break;
      }

      if (r != 2) {
        fprintf(stderr, "Invalid input format\n");
        free(items);
        return 1;
      }

      if (n >= items_size) {
        items_size *= 2;
        items = realloc(items, items_size * sizeof(Item));
      }

      items[n++] = item;
    }

    KnapsackSolution solution = knapsack(items, n, capacity);
    printKnapsackSolution(solution);

    free(solution.selected);
  }
}
#endif
