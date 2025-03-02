# Testing code on example of Implementing an `interval_map` in C++

## Introduction

In this article, I'll walk through my solution to an interesting C++ programming challenge: implementing the `assign` method for an `interval_map` data structure. This challenge tests fundamental understanding of data structures, C++ STL, and efficient algorithm design. Let's dive into the problem and explore a robust solution.

## Understanding the Problem

An `interval_map<K,V>` is a specialized data structure that associates ranges of consecutive keys of type K with values of type V. It's particularly useful when multiple consecutive keys should map to the same value, allowing for a more compact representation than a regular map.

The implementation builds on top of `std::map`, where each entry `(k,v)` indicates that the value `v` is associated with all keys from `k` (inclusive) to the next key in the map (exclusive). Additionally, a special value `m_valBegin` represents the value for all keys less than the first key in the map.

For example, given an `interval_map<int,char>` with:
- `m_valBegin = 'A'`
- `m_map = {(1,'B'), (3,'A')}`

The mapping would be:
- All keys less than 1 → 'A'
- Keys 1 and 2 → 'B'
- All keys 3 and greater → 'A'

The representation must be canonical, meaning consecutive map entries cannot contain the same value, and the first map entry must differ from `m_valBegin`.

## The `assign` Method Challenge

Our task is to implement the `assign(keyBegin, keyEnd, val)` method, which assigns the value `val` to all keys in the half-open interval `[keyBegin, keyEnd)`. This operation may require inserting new entries into the map or modifying existing ones while maintaining the canonical representation.

## Key Constraints

The solution must handle these constraints:
- The key type `K` supports only copying, moving, and comparison via `operator<`
- The value type `V` supports copying, moving, and equality comparison via `operator==`
- The method must handle empty intervals (where `!(keyBegin < keyEnd)`) by doing nothing
- The implementation must maintain the canonical representation

## Solution Approach

My approach follows these steps:

1. Check for an empty interval and return early if detected
2. Find the affected range in the map
3. Determine the values before and after the affected range
4. Clear the affected range
5. Insert new entries at the boundaries if needed
6. Clean up to maintain the canonical representation


## Complete Implementation

Here's the complete implementation of the `interval_map` class including the `assign` method code with test in this repository.


## Testing the Implementation

Comprehensive testing is crucial to ensure the correctness of our `interval_map` implementation. Based on the provided test cases, I've developed a testing strategy that covers various scenarios:

### 1. Basic Functionality Tests

These tests verify that the fundamental operations work correctly:

```cpp
void test_simple_assign() {
    interval_map<int, char> m('A');

    // Basic assignment
    m.assign(1, 3, 'B');
    assert(m.size() == 2); // Entry at 1 and 3
    assert(m[0] == 'A');
    assert(m[1] == 'B');
    assert(m[2] == 'B');
    assert(m[3] == 'A');
    assert(m.is_canonical());
}
```

### 2. Empty Interval Handling

These tests ensure that empty intervals are properly handled according to the specification:

```cpp
void test_empty_interval() {
    interval_map<int, char> m('A');

    // Empty interval (keyBegin >= keyEnd)
    m.assign(5, 5, 'B'); // Equal keys
    assert(m.size() == 0);
    assert(m['A'] == 'A');

    m.assign(10, 5, 'B'); // keyBegin > keyEnd
    assert(m.size() == 0);
    assert(m['A'] == 'A');
}
```

### 3. Overlapping Intervals

These tests verify correct behavior when assigning to intervals that overlap with existing ones:

```cpp
void test_overlapping_intervals() {
    interval_map<int, char> m('A');

    // First assignment
    m.assign(10, 20, 'B');
    
    // Overlapping assignment that extends right
    m.assign(15, 25, 'C');
    assert(m[9] == 'A');
    assert(m[10] == 'B');
    assert(m[14] == 'B');
    assert(m[15] == 'C');
    assert(m[24] == 'C');
    assert(m[25] == 'A');
    assert(m.is_canonical());

    // Overlapping assignment that extends left
    m.assign(5, 15, 'D');
    assert(m[4] == 'A');
    assert(m[5] == 'D');
    assert(m[14] == 'D');
    assert(m[15] == 'C');
    assert(m[24] == 'C');
    assert(m[25] == 'A');
    assert(m.is_canonical());

    // Completely overlapping assignment
    m.assign(0, 30, 'E');
    assert(m[-1] == 'A');
    assert(m[0] == 'E');
    assert(m[29] == 'E');
    assert(m[30] == 'A');
    assert(m.is_canonical());
}
```

### 4. Canonicity Maintenance

These tests ensure that the map maintains its canonical representation:

```cpp
void test_canonicity() {
    interval_map<int, char> m('A');

    // Assign same value as m_valBegin
    m.assign(1, 5, 'A');
    assert(m.size() == 0); // No entries should be added
    
    // Create a non-trivial map
    m.assign(10, 20, 'B');
    m.assign(30, 40, 'C');
    
    // Assign with same value as adjacent region
    m.assign(20, 30, 'B');
    assert(m.size() == 3); // Should merge regions
    assert(m[10] == 'B');
    assert(m[29] == 'B'); // Merged region
    assert(m[30] == 'C');
    assert(m.is_canonical());
}
```

### 5. Boundary Conditions

These tests verify behavior at the exact boundaries of existing intervals:

```cpp
void test_boundary_conditions() {
    interval_map<int, char> m('A');

    // Assign at exact boundaries of existing interval
    m.assign(10, 20, 'B');
    m.assign(10, 15, 'C');
    assert(m[10] == 'C');
    assert(m[14] == 'C');
    assert(m[15] == 'B');
    
    // Assign same value with different boundary
    m.assign(15, 25, 'B');
    assert(m[15] == 'B');
    assert(m[24] == 'B');
    assert(m[25] == 'A');
    assert(m.is_canonical());
}
```

### 6. Complex Scenarios

These tests combine multiple operations to test various edge cases:

```cpp
void test_complex_scenarios() {
    interval_map<int, char> m('D');

    m.assign(10, 20, '1');
    m.assign(30, 40, '2');

    // Assign over multiple regions
    m.assign(15, 35, 'O');
    assert(m[9] == 'D');
    assert(m[10] == '1');
    assert(m[14] == '1');
    assert(m[15] == 'O');
    assert(m[34] == 'O');
    assert(m[35] == '2');
    assert(m[39] == '2');
    assert(m[40] == 'D');
    assert(m.is_canonical());

    // Create a complex map with many regions
    interval_map<int, char> m2('A');
    m2.assign(10, 20, 'B');
    m2.assign(30, 40, 'C');
    m2.assign(50, 60, 'D');
    m2.assign(70, 80, 'E');

    // Make complex overlapping assignments
    m2.assign(15, 35, 'F');
    m2.assign(45, 75, 'G');
    m2.assign(5, 85, 'H');

    assert(m2[4] == 'A');
    assert(m2[5] == 'H');
    assert(m2[84] == 'H');
    assert(m2[85] == 'A');
    assert(m2.is_canonical());
}
```

### 7. Different Value Types

The implementation should work with different value types, not just characters:

```cpp
void test_complex_scenarios_str() {
    // Test with string values
    interval_map<int, std::string> m(std::string("default"));

    std::string region1("region1");
    std::string region2("region2");
    std::string overlap("overlap");

    m.assign(10, 20, region1);
    m.assign(30, 40, region2);
    m.assign(15, 35, overlap);
    
    assert(m[9] == "default");
    assert(m[10] == "region1");
    assert(m[15] == "overlap");
    assert(m[34] == "overlap");
    assert(m[35] == "region2");
    assert(m.is_canonical());
}
```

### 8. Randomized Testing

To thoroughly test the implementation, randomized testing is employed:

```cpp
// Randomized tests
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> key_dist(-100, 100);
std::uniform_int_distribution<> val_dist(0, 25);

for (int i = 0; i < 1000; ++i) {
    interval_map<int, char> random_map('A');
    std::vector<std::tuple<int, int, char>> assignments;
    for (int j = 0; j < 100; ++j) {
        int begin = key_dist(gen);
        int end = key_dist(gen);
        if (begin > end)
            std::swap(begin, end);
        char value = 'A' + val_dist(gen);
        assignments.emplace_back(begin, end, value);
    }
    std::shuffle(assignments.begin(), assignments.end(), gen);
    for (const auto &[begin, end, value] : assignments) {
        random_map.assign(begin, end, value);
        assert(random_map.is_canonical());
    }
}
```

## Testing Strategy Overview

Our comprehensive testing approach includes:

1. **Unit Tests**: Verifying basic functionality and edge cases
2. **Property Tests**: Ensuring the canonical representation is always maintained
3. **Regression Tests**: Testing specific scenarios that could trigger bugs
4. **Randomized Tests**: Using random inputs to discover unexpected issues

This thorough testing strategy ensures the `interval_map` implementation is robust and reliable across a wide range of scenarios.

## Edge Cases and Optimizations

When implementing this solution, I considered several edge cases:

1. **Empty map**: The code handles an initially empty map correctly.
2. **Boundary insertions**: We only insert new entries when there's a value change at the boundaries.
3. **Overlapping intervals**: The solution correctly handles cases where the new interval overlaps with existing intervals.
4. **Memory efficiency**: By avoiding unnecessary insertions, we keep the map as small as possible.

## Time and Space Complexity

The time complexity of the `assign` method is:
- O(log n) for finding the affected range boundaries using `lower_bound`
- O(k + log n) for erasing k elements in the affected range
- O(log n) for each of the two potential insertions
- O(m) for cleaning up, where m is the number of consecutive entries with identical values

The space complexity is O(1) for the additional variables used.

## Conclusion

The `interval_map` data structure provides an elegant solution for scenarios where ranges of keys should map to the same value. The implementation of the `assign` method demonstrates several important C++ concepts:

1. Efficient use of STL containers and algorithms
2. Careful boundary condition handling
3. Maintaining invariants in data structures
4. Balancing performance with code clarity

This solution maintains the canonical representation while efficiently handling the assignment operation, making it a robust implementation for this useful data structure.

The extensive testing confirms that our implementation correctly handles all edge cases and maintains the canonical representation under various conditions, ensuring reliability in real-world applications.
