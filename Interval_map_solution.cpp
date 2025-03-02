#include <iostream>
#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <type_traits>

// Include the interval_map implementation
template <typename K, typename V>
class interval_map
{
    friend void IntervalMapTest(); // Friend function for testing
    V m_valBegin;                  // Initial value for the map
    std::map<K, V> m_map;          // The map storing intervals

public:
    // Constructor that associates the entire range of K with the value val
    template <typename V_forward>
    interval_map(V_forward &&val)
        : m_valBegin(std::forward<V_forward>(val)) {}

    // Assign the value val to the interval [keyBegin, keyEnd)
    template <typename V_forward>
    void assign(const K &keyBegin, const K &keyEnd, V_forward &&val)
        requires std::is_same_v<std::remove_cvref_t<V_forward>, V>
    {
        std::cout << "Assigning: " << val << "[" << keyBegin << ":" << keyEnd << "]" << std::endl;

        // If the interval is invalid, return
        if (!(keyBegin < keyEnd))
        {
            this->print();
            return;
        }

        // If the map is empty, insert the first interval
        if (m_map.empty())
        {
            if (val != m_valBegin)
            {
                m_map.insert_or_assign(keyEnd, m_valBegin);
                m_map.insert_or_assign(keyBegin, std::forward<V_forward>(val));
                this->print();
            }
            return;
        }

        K last_key = (m_map.rbegin()->first < keyEnd) ? keyEnd : m_map.rbegin()->first;
        {
            std::pair<K, V> *resized = nullptr;
            std::vector<typename std::map<K, V>::iterator> will_delete;

            // Find the first element whose key is not less than keyEnd
            auto itBegin = m_map.lower_bound(keyBegin);
            auto itEnd = m_map.lower_bound(keyEnd);
            auto iterator_end = (itEnd == m_map.end() ? itEnd : std::next(itEnd));

            // Iterate over the range and delete overlapping records
            for (auto it = itBegin; it != iterator_end; ++it)
            {
                // existing interval is inside new
                // started location as new
                // if (it->first == keyBegin && it->second == val)
                if (!(keyBegin < it->first) && !(it->first < keyBegin) && it->second == val)
                {
                    auto next_it = std::next(it);
                    if (next_it != m_map.end() && next_it->second == val)
                    {
                        will_delete.push_back(next_it);
                    }
                }
                // started location inside new
                else if (!(keyEnd < it->first) && it->second == val)
                {
                    will_delete.push_back(it);
                }
                else
                {
                    // if the interval hits the new boundary
                    // if (it->first >= keyBegin && it->first < keyEnd)
                    if (!(it->first < keyBegin) && it->first < keyEnd)
                    {
                        auto next_it = std::next(it);
                        if (next_it != m_map.end() && keyEnd < next_it->first)
                        {
                            resized = new std::pair(keyEnd, it->second);
                        }
                        will_delete.push_back(it);
                    }
                }
            }

            // Insert resized interval if needed
            if (resized != nullptr)
            {
                m_map.insert_or_assign(resized->first, std::forward<V_forward>(resized->second));
            }

            // Delete overlapping records
            for (auto it : will_delete)
            {
                m_map.erase(it);
            }
        }

        // Insert the new interval
        {
            auto insert = m_map.insert_or_assign(keyBegin, std::forward<V_forward>(val)).first;
            if (insert != m_map.end())
            {
                auto what_after = std::next(insert);
                // Looking for same sequential intervals left
                if (what_after->second == val)
                {
                    m_map.erase(what_after);
                }
            }

            if (insert != m_map.begin())
            {
                auto what_before = std::prev(insert);
                // Looking for same sequential intervals right
                if (what_before->second == val)
                {
                    m_map.erase(insert);
                }
            }
        }

        // Canonization: remove valBegin from the map top
        while (!m_map.empty() && m_map.begin()->second == m_valBegin)
        {
            m_map.erase(m_map.begin());
        }

        // Remove only m_valBegin sequential intervals
        if (!m_map.empty())
        {
            auto last_map_item = std::prev(m_map.end());
            if (last_map_item->second == m_valBegin)
            {
                if (m_map.size() > 1)
                {
                    auto last_map_item_1 = std::prev(last_map_item);
                    if (last_map_item_1->second == m_valBegin)
                    {
                        // If the full map is m_valBegin
                        if (m_map.size() == 2)
                            m_map.erase(last_map_item_1);
                        m_map.erase(last_map_item);
                    }
                }
                else
                {
                    // Remove only one m_valBegin interval
                    m_map.erase(last_map_item);
                }
            }
        }

        // If the map is empty, return
        if (m_map.empty())
        {
            this->print();
            return;
        }

        // If the last value in the map is not m_valBegin, set it
        if (!(m_map.rbegin()->second == m_valBegin))
        {
            m_map[last_key] = m_valBegin;
        }

        this->print();
    }

    // Look up the value associated with a key
    const V &operator[](const K &key) const
    {
        auto it = m_map.upper_bound(key);
        if (it == m_map.begin())
        {
            return m_valBegin;
        }
        else
        {
            return (--it)->second;
        }
    }

    // Check if the internal representation is canonical
    bool is_canonical() const
    {
        if (m_map.empty())
            return true;

        if (m_map.begin()->second == m_valBegin)
        {
            return false;
        }

        auto it = m_map.begin();
        auto prev_value = it->second;
        ++it;

        while (it != m_map.end())
        {
            if (it->second == prev_value)
            {
                return false;
            }
            prev_value = it->second;
            ++it;
        }

        return true;
    }

    // Get the internal map size
    size_t size() const
    {
        return m_map.size();
    }

    // Print the map contents
    void print() const
    {
        std::cout << "m_valBegin: " << m_valBegin << std::endl;
        std::cout << "Map contents:" << std::endl;
        for (const auto &[key, value] : m_map)
        {
            std::cout << key << " -> " << value << std::endl;
        }
        std::cout << std::endl;
    }

    // Get the initial value of the map
    const V get_valBegin()
    {
        return m_valBegin;
    }
};

// Test cases
void test_empty_interval()
{
    std::cout << "Testing empty interval..." << std::endl;
    interval_map<int, char> m('A');

    // Empty interval (keyBegin >= keyEnd)
    m.assign(5, 5, 'B'); // Equal keys
    assert(m.size() == 0);
    assert(m['A'] == 'A');

    m.assign(10, 5, 'B'); // keyBegin > keyEnd
    assert(m.size() == 0);
    assert(m['A'] == 'A');

    std::cout << "Empty interval tests passed!" << std::endl;
}

void test_simple_assign()
{
    std::cout << "Testing simple assignment..." << std::endl;
    interval_map<int, char> m('A');

    // Basic assignment
    m.assign(1, 3, 'B');
    assert(m.size() == 2); // Entry at 1 and 3
    assert(m[0] == 'A');
    assert(m[1] == 'B');
    assert(m[2] == 'B');
    assert(m[3] == 'A');
    assert(m.is_canonical());

    std::cout << "Simple assignment tests passed!" << std::endl;
}

void test_overlapping_intervals()
{
    std::cout << "Testing overlapping intervals..." << std::endl;
    interval_map<int, char> m('A');

    // First assignment
    m.assign(10, 20, 'B');
    assert(m[9] == 'A');
    assert(m[10] == 'B');
    assert(m[19] == 'B');
    assert(m[20] == 'A');

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

    std::cout << "Overlapping intervals tests passed!" << std::endl;
}

void test_canonicity()
{
    std::cout << "Testing canonicity maintenance... m" << std::endl;
    interval_map<int, char> m('A');

    // Assign same value as m_valBegin
    m.assign(1, 5, 'A');
    assert(m.size() == 0); // No entries should be added
    assert(m[0] == 'A');
    assert(m[1] == 'A');
    assert(m[4] == 'A');
    assert(m[5] == 'A');
    assert(m.is_canonical());

    // Create a non-trivial map
    m.assign(10, 20, 'B');
    m.assign(30, 40, 'C');
    m.assign(1, 5, 'A');
    assert(m.size() == 4); // Entries at 10, 20, 30, 40
    m.assign(32, 34, 'A');

    // Assign with same value as adjacent region
    m.assign(20, 30, 'B');
    assert(m.size() == 3); // Should merge regions and have entries at 10, 40
    assert(m[9] == 'A');
    assert(m[10] == 'B');
    assert(m[20] == 'B'); // Same as previous
    assert(m[29] == 'B');
    assert(m[30] == 'C'); // Merged
    assert(m[39] != 'C');
    assert(m[40] == 'A');
    assert(m.is_canonical());

    // Reset map
    std::cout << "Testing canonicity maintenance... m2" << std::endl;
    interval_map<int, char> m2('A');

    // Create a map with multiple regions
    m2.assign(10, 20, 'B');
    m2.assign(20, 30, 'C');
    m2.assign(30, 40, 'D');

    // Assign same value across multiple regions
    m2.assign(15, 35, 'E');
    assert(m2.is_canonical());
    assert(m2[10] == 'B');
    assert(m2[14] == 'B');
    assert(m2[15] == 'E');
    assert(m2[34] == 'E');
    assert(m2[35] == 'D');

    std::cout << "Canonicity tests passed!" << std::endl;
}

void test_boundary_conditions()
{
    std::cout << "Testing boundary conditions..." << std::endl;
    interval_map<int, char> m('A');

    // Assign at exact boundaries of existing interval
    m.assign(10, 20, 'B');
    m.assign(10, 15, 'C');
    assert(m[9] == 'A');
    assert(m[10] == 'C');
    assert(m[14] == 'C');
    assert(m[15] == 'B');
    assert(m[19] == 'B');
    assert(m[20] == 'A');
    assert(m.is_canonical());

    // Assign same value with different boundary
    m.assign(15, 25, 'B');
    assert(m[9] == 'A');
    assert(m[10] == 'C');
    assert(m[14] == 'C');
    assert(m[15] == 'B');
    assert(m[24] == 'B');
    assert(m[25] == 'A');
    assert(m.is_canonical());

    std::cout << "Boundary condition tests passed!" << std::endl;
}

void test_complex_scenarios()
{
    std::cout << "Testing complex scenarios..." << std::endl;

    // Test with string values - use actual std::string objects, not string literals
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

    // Assign with same values at edges to test canonicity
    m.assign(5, 15, '1');
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

    std::cout << "Complex scenario tests passed!" << std::endl;
}

void test_complex_scenarios_str()
{
    std::cout << "Testing complex scenarios strings..." << std::endl;

    // Test with string values - use actual std::string objects, not string literals
    interval_map<int, std::string> m(std::string("default"));

    std::string region1("region1");
    std::string region2("region2");
    std::string overlap("overlap");

    m.assign(10, 20, region1);
    m.assign(30, 40, region2);

    // Assign over multiple regions
    m.assign(15, 35, overlap);
    assert(m[9] == "default");
    assert(m[10] == "region1");
    assert(m[14] == "region1");
    assert(m[15] == "overlap");
    assert(m[34] == "overlap");
    assert(m[35] == "region2");
    assert(m[39] == "region2");
    assert(m[40] == "default");
    assert(m.is_canonical());

    // Assign with same values at edges to test canonicity
    m.assign(5, 15, region1);
    assert(m.is_canonical());
}

void test_complex_scenarios_str2()
{
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
    m2.assign(75, 80, 'D');

    assert(m2[4] == 'A');
    assert(m2[5] == 'H');
    assert(m2[74] == 'H');
    assert(m2[85] == 'A');
    assert(m2[77] == 'D');
    assert(m2.is_canonical());

    std::cout << "Complex scenario tests passed!" << std::endl;
}

void IntervalMapTest()
{
    // Create an interval_map with initial value 'A'
    interval_map<int, char> imap('A');

    // Check initial state
    assert(imap[0] == 'A');
    assert(imap[10] == 'A');
    assert(imap.size() == 0);

    // Assign value 'B' to interval [1, 5)
    imap.assign(1, 5, 'B');
    assert(imap[0] == 'A');
    assert(imap[1] == 'B');
    assert(imap[4] == 'B');
    assert(imap[5] == 'A');
    assert(imap.size() == 2);

    // Assign value 'C' to interval [3, 7)
    imap.assign(3, 7, 'C');
    assert(imap[2] == 'B');
    assert(imap[3] == 'C');
    assert(imap[6] == 'C');
    assert(imap[7] == 'A');
    assert(imap.size() == 3);

    // Assign value 'A' to interval [2, 4)
    imap.assign(2, 4, 'A');
    assert(imap[1] == 'B');
    assert(imap[2] == 'A');
    assert(imap[3] == 'A');
    assert(imap[4] == 'C');
    assert(imap.size() == 4);

    // Test is_canonical method
    assert(imap.is_canonical() == true);

    // Print the map contents
    imap.print();
}

#include <random>

void IntervalMapTest_G()
{
    interval_map<int, char> m('A');
    assert(m.get_valBegin() == 'A');
    assert(m.size() == 0);
    assert(m.is_canonical());

    m.assign(1, 3, 'B');
    assert(m[0] == 'A');
    assert(m[1] == 'B');
    assert(m[2] == 'B');
    assert(m[3] == 'A');
    assert(m.is_canonical());
    assert(m.size() == 2);
    assert(m[1] == 'B');
    assert(m[3] == 'A');

    m.assign(3, 5, 'C');
    assert(m[3] == 'C');
    assert(m[4] == 'C');
    assert(m[5] == 'A');
    assert(m.is_canonical());
    assert(m.size() == 3);
    assert(m[1] == 'B');
    assert(m[3] == 'C');
    assert(m[5] == 'A');

    m.assign(2, 4, 'A');
    assert(m[1] == 'B');
    assert(m[2] == 'A');
    assert(m[3] == 'A');
    assert(m[4] == 'C');
    assert(m.is_canonical());
    assert(m.size() == 4);
    assert(m[1] == 'B');
    assert(m[2] == 'A');
    assert(m[4] == 'C');

    m.assign(0, 10, 'D');
    assert(m[0] == 'D');
    assert(m[5] == 'D');
    assert(m.is_canonical());
    assert(m.size() == 2);
    assert(m.get_valBegin() == 'A');

    m.assign(0, 10, 'A');
    assert(m[0] == 'A');
    assert(m[5] == 'A');
    assert(m.is_canonical());
    assert(m.size() == 0);
    assert(m.get_valBegin() == 'A');

    m.assign(5, 7, 'B');
    assert(m[4] == 'A');
    assert(m[5] == 'B');
    assert(m[6] == 'B');
    assert(m[7] == 'A');
    assert(m.is_canonical());
    assert(m.size() == 2);
    assert(m[5] == 'B');
    assert(m[7] == 'A');

    m.assign(6, 6, 'C');
    assert(m.is_canonical());
    assert(m.size() == 2);

    m.assign(7, 10, 'B');
    assert(m[6] == 'B');
    assert(m[7] == 'B');
    assert(m.is_canonical());
    assert(m.size() == 2);
    assert(m[5] == 'B');

    m.assign(0, 5, 'B');
    assert(m[0] == 'B');
    assert(m.is_canonical());
    assert(m.size() == 2);

    m.assign(0, 10, 'A');
    assert(m.is_canonical());
    assert(m.size() == 0);

    // Randomized tests
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> key_dist(-100, 100);
    std::uniform_int_distribution<> val_dist(0, 25);

    for (int i = 0; i < 1000; ++i)
    {
        interval_map<int, char> random_map('A');
        std::vector<std::tuple<int, int, char>> assignments;
        for (int j = 0; j < 100; ++j)
        {
            int begin = key_dist(gen);
            int end = key_dist(gen);
            if (begin > end)
                std::swap(begin, end);
            char value = 'A' + val_dist(gen);
            assignments.emplace_back(begin, end, value);
        }
        std::shuffle(assignments.begin(), assignments.end(), gen);
        for (const auto &[begin, end, value] : assignments)
        {
            random_map.assign(begin, end, value);
            assert(random_map.is_canonical());
        }
    }
}

void t()
{
    {
        interval_map<int, char> m('A');
        m.assign(-89, -87, 'M');
        m.assign(-87, -68, 'T');
        m.assign(-68, -57, 'F');
        m.assign(-57, -39, 'L');
        m.assign(-39, -37, 'P');
        m.assign(-37, 41, 'O');
        m.assign(41, 91, 'I');
        m.assign(91, 93, 'R');
        m.assign(93, 100, 'A');

        // Asserting R[55:81]
        m.assign(55, 81, 'R');
        assert(m.is_canonical());

        // Optional assertions to verify the map's state after the assignment
        assert(m.size() == 9);
        assert(m[-89] == 'M');
        assert(m[-87] == 'T');
        assert(m[-68] == 'F');
        assert(m[-57] == 'L');
        assert(m[-39] == 'P');
        assert(m[-37] == 'O');
        assert(m[55] == 'R');
        assert(m[47] == 'I');
    }
    {
        interval_map<int, char> m('A');
        m.assign(-100, -94, 'Y');
        m.assign(-94, -85, 'T');
        m.assign(-85, -33, 'B');
        m.assign(-33, 49, 'K');
        m.assign(49, 91, 'F');

        // Asserting T[-100:-95]
        m.assign(-100, -95, 'T');
        assert(m.is_canonical());

        // Optional assertions to verify the map's state after the assignment
        assert(m.size() == 7);
        assert(m[-100] == 'T');
        assert(m[-95] == 'Y');
        assert(m[-94] == 'T');
        assert(m[-85] == 'B');
        assert(m[-33] == 'K');
        assert(m[49] == 'F');
    }

    {
        interval_map<int, char> m('A');
        m.assign(-96, -95, 'X');
        m.assign(-95, -79, 'I');
        m.assign(-79, -66, 'P');
        m.assign(-66, 62, 'G');
        m.assign(62, 68, 'E');
        m.assign(68, 80, 'G');
        m.assign(80, 87, 'K');
        m.assign(87, 89, 'T');
        m.assign(89, 94, 'I');
        m.assign(94, 99, 'B');
        m.assign(99, 100, 'A');

        // Asserting E[-66:-1]
        m.assign(-66, -1, 'E');
        assert(m.is_canonical());

        // Optional assertions to verify the map's state after the assignment
        assert(m.size() == 12);
        assert(m[-96] == 'X');
        assert(m[-95] == 'I');
        assert(m[-79] == 'P');
        assert(m[-66] == 'E');
        assert(m[-1] == 'G');
        assert(m[62] == 'E');
        assert(m[68] == 'G');
        assert(m[80] == 'K');
    }
    {
        interval_map<int, char> m('A');
        m.assign(-96, -6, 'Y');
        m.assign(-6, 31, 'E');
        m.assign(31, 97, 'N');
        m.assign(97, 100, 'E');
        m.assign(100, 101, 'A');

        m.assign(-86, 14, 'N');
        assert(m.is_canonical());

        // Optional: Verify the map contents after the assignment
        assert(m.size() == 6);
        assert(m[-96] == 'Y'); // Replace m.get_map().at(-96) with m[-96]
        assert(m[-86] == 'N');
        assert(m[14] == 'E');
    }

    {
        interval_map<int, char> m('A');

        // m.assign(-95, -89, 'M');
        // m.assign(-89, -88, 'C');
        m.assign(-88, -36, 'V');
        m.assign(-36, 30, 'Z');
        m.assign(30, 62, 'G');
        m.assign(62, 95, 'B');
        m.assign(95, 100, 'A');
    }
    {
        interval_map<int, char> m('A');

        m.assign(-100, -96, 'S');
        m.assign(-96, -30, 'X');
        m.assign(-30, -27, 'I');
        m.assign(-27, -1, 'O');
        m.assign(-1, 46, 'J');
        m.assign(46, 85, 'N');
        m.assign(85, 89, 'R');
        m.assign(89, 90, 'J');
        m.assign(90, 96, 'F');
        m.assign(96, 98, 'V');
        m.assign(98, 99, 'H');
        m.assign(99, 100, 'K');
        m.assign(100, 101, 'A');

        m.assign(44, 85, 'R');
        assert(m.is_canonical());
    }
    {
        interval_map<int, char> m('A');

        m.assign(-98, -97, 'F');
        m.assign(-97, -93, 'I');
        m.assign(-93, -31, 'V');
        m.assign(-31, -28, 'G');
        m.assign(-28, 89, 'F');
        m.assign(89, 94, 'U');
        m.assign(94, 95, 'R');
        // m.assign(95, 100, 'A');

        m.assign(-97, 56, 'H');
        assert(m.is_canonical());
    }
    {
        interval_map<int, char> m('A');

        m.assign(-96, -87, 'G');
        m.assign(-87, -74, 'Q');
        m.assign(-74, -68, 'O');
        m.assign(-68, -19, 'E');
        m.assign(-19, 28, 'Z');
        m.assign(28, 71, 'F');
        m.assign(71, 76, 'U');
        m.assign(76, 80, 'R');
        m.assign(80, 90, 'U');
        m.assign(90, 92, 'T');
        m.assign(92, 96, 'L');
        m.assign(96, 97, 'T');
        m.assign(97, 100, 'A');

        m.assign(-98, 4, 'G');
        assert(m.is_canonical());
    }
    {
        interval_map<int, char> m('A');

        m.assign(-91, -90, 'M');
        m.assign(-90, -85, 'P');
        m.assign(-85, -64, 'M');
        m.assign(-64, -63, 'K');
        m.assign(-63, -44, 'S');
        m.assign(-44, -24, 'B');
        m.assign(-24, 81, 'Y');
        m.assign(81, 100, 'A');

        m.assign(-28, 24, 'A');
        assert(m.is_canonical());
    }

    {
        interval_map<int, char> m('A');

        m.assign(-95, -94, 'O');
        m.assign(-94, -90, 'U');
        m.assign(-90, -57, 'Z');
        m.assign(-57, -52, 'O');
        m.assign(-52, -8, 'E');
        m.assign(-8, 27, 'I');
        m.assign(27, 94, 'A');
        m.assign(94, 100, 'T');
        m.assign(100, 101, 'A');

        m.assign(23, 68, 'T');
        assert(m.is_canonical());
    }
}

int main()
{
    std::cout << "=== Starting interval_map tests ===" << std::endl;

    test_complex_scenarios_str2();

    t();

    IntervalMapTest_G();

    test_complex_scenarios_str();
    test_complex_scenarios();
    IntervalMapTest();
    test_empty_interval();
    test_simple_assign();
    test_overlapping_intervals();
    test_canonicity();
    test_boundary_conditions();

    std::cout << "=== All tests passed! ===" << std::endl;
    return 0;
}