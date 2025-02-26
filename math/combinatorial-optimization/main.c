#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Item {
  int weight;
  int value;
} Item;

Item *knapsack(Item items[], int n, int capacity, int *max_value,
               int *remaining_capacity, int *n_items) {
  if (n == 0 || capacity == 0) {
    *max_value = 0;
    *remaining_capacity = capacity;
    *n_items = 0;
    return NULL;
  }

  if (n==1) {
    Item *selected_items = (Item *)malloc(1 * sizeof(Item));
    if (items[0].weight <= capacity) {
      selected_items[0] = items[0];
    } else {
      selected_items[0].weight = 0;
      selected_items[0].value = 0;
    }
    *max_value = selected_items[0].value;
    *remaining_capacity = capacity - selected_items[0].weight;
    *n_items = 1;
    return selected_items;
  }

  // Create a 2D array to store the maximum value for each capacity
  int **dp = (int **)malloc((n + 1) * sizeof(int *));
  for (int i = 0; i <= n; i++) {
    dp[i] = (int *)malloc((capacity + 1) * sizeof(int));
  }

  // Build the table
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

  *max_value = dp[n][capacity];
  *remaining_capacity = w;
  *n_items = count;

  return selected_items;
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

bool testArrayEqual(Item *actual, Item *expected, int size) {
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

void printTest(const char *test_name, Item *actual, int actual_size,
                Item *expected, int expected_size, int actual_max_value,
                int expected_max_value, int actual_remaining_capacity,
                int expected_remaining_capacity) {
  printf("Test: %s\n", test_name);
  printf("Actual items:\n");
  for (int i = 0; i < actual_size; i++) {
    printf("Item %d: Weight = %d, Value = %d\n", i + 1, actual[i].weight,
           actual[i].value);
  }
  printf("Expected items:\n");
  for (int i = 0; i < expected_size; i++) {
    printf("Item %d: Weight = %d, Value = %d\n", i + 1, expected[i].weight,
           expected[i].value);
  }
  printf("Actual max value: %d\n", actual_max_value);
  printf("Expected max value: %d\n", expected_max_value);
  printf("Actual remaining capacity: %d\n", actual_remaining_capacity);
  printf("Expected remaining capacity: %d\n", expected_remaining_capacity);
}

void checkTest(const char *test_name, Item *actual, int actual_size,
                Item *expected, int expected_size, int actual_max_value,
                int expected_max_value, int actual_remaining_capacity,
                int expected_remaining_capacity) {
  if (!testEqual(actual_size, expected_size) || !testEqual(actual_max_value, expected_max_value) ||
      !testEqual(actual_remaining_capacity, expected_remaining_capacity) ||
      !testArrayEqual(actual, expected, actual_size)) {
    printTest(test_name, actual, actual_size, expected, expected_size,
              actual_max_value, expected_max_value, actual_remaining_capacity,
              expected_remaining_capacity);
    printf("Test failed!\n");
    exit(1);
  } else {
    printf("Test passed!\n");
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
  }

  printf("All test cases passed!\n");
}

#else
int main() {
  int capacity = 50;
  Item items[] = {{10, 60}, {20, 100}, {20, 30}};
  int n = sizeof(items) / sizeof(items[0]);

  int max_value;
  int remaining_capacity;
  int n_items;
  Item *selection = knapsack(items, n, capacity, &max_value, &remaining_capacity,
                             &n_items);

  printf("Maximum value in knapsack = %d\n", max_value);
  printf("Remaining capacity = %d\n", remaining_capacity);
  printf("Number of items included = %d\n", n_items);
  printf("Items included in the knapsack:\n");
  for (int i = 0; i < n_items; i++) {
    printf("Item %d: Weight = %d, Value = %d\n", i + 1, selection[i].weight,
           selection[i].value);
  }

  free(selection);
}
#endif
