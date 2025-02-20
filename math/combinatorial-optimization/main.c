#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Item {
  int weight;
  int value;
} Item;

void knapsack(Item items[], int n, int capacity, int *max_value,
               int *remaining_capacity, int *n_items) {
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

  *max_value = dp[n][capacity];
  *remaining_capacity = w;
  *n_items = count;
}
int main() {
}
