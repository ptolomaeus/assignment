#include <cstdio>
#include <algorithm>
#include <utility>
#include <functional>
#include <queue>
#include <stack>
#include <cmath>
#include <iterator>
#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <random>
#include <map>
#include <iomanip>
#include <stdlib.h>
#include <list>
#include <typeinfo>
#include <cassert>
#include <fstream>
#include <unordered_map>
#include <cstdlib>
#include <complex>
#include <cctype>
#include <bitset>
#include <tuple>
#include <ctime>
using namespace std;

using ll = int;
using vll = vector<ll>;
using pll = pair<ll, ll>;
using qll = queue<ll>;
using vb = vector<bool>;
using mll = map<ll, ll>;
using sll = stack<ll>;
#define REP(i,n) for(ll i(0);(i)<(n);(i)++)
#define rep(i,n) for(ll i(0);(i)<(n);(i)++)
#define ALL(a) a.begin(), a.end()
#define enld endl //* missspell check

struct Item
{
	int index;
	int value;
	int weight;

	// high value density item will come first
	bool operator < (Item rhs) const {
		return static_cast<double>(value) / weight > static_cast<double>(rhs.value) / rhs.weight;
	}
};

auto solveByDp(vector<Item> &items, int capacity){
    //* convert api
    ll N = items.size(), K = capacity;
    vector<int>  v(N), w(N);
    REP(i, N){
        v[i] = items[i].value;
        w[i] = items[i].weight;
    }
    //* build dynamic programming table
    vector<vll> dp(N + 1, vll(K + 1, 0));
    //* update dp table
    REP(i, N){
        REP(j, K + 1){
            dp[i + 1][j] = dp[i][j];
            if(j >= w[i]){
                dp[i + 1][j] = max(dp[i + 1][j], dp[i][j - w[i]] + v[i]);
            }
        }
    }
    //* recover solution from dp table
    vll res(N, 0);
    ll k = K;
    for(ll i = N - 1 ; i >= 0; i--){
        if(w[i] <= k && dp[i][k - w[i]] + v[i] >= dp[i][k]){
            res[i] = 1;
            k -= w[i];
        }
    }
    //* output
    return make_tuple(dp[N][K], res);
}

// get the max value expectation from current capacity and current undecided item
auto get_expectation(const vector<Item> & items, int capacity, int start)
{
	auto expectation = 0.0;
	for(auto i = start; i < items.size(); ++i)
	{
		auto item = items[i];
		if(capacity >= item.weight)
		{
			expectation += item.value;
			capacity -= item.weight;
		}
		// if current capacity is not enough to carry the whole item, then put a fraction of it into the knapsack
		// and add the same fraction of its value to the expectation
		else
		{
			expectation += static_cast<double>(item.value) * capacity / item.weight;
			break;
		}
	}

	return expectation;
}

// find max value and the take/no-take choice for each item
auto search(const vector<Item> & items,  int capacity)
{
	auto max_value = 0.0;
	auto max_taken = vector<int>(items.size(), 0);

	// to prevent from stack-overflow, instead of using plain recursion here I maintain the stack myself
	// a stack element includes 5 parts:
	// value:         value accumulated so far
	// capacity:      left capacity
	// expectation:   upper bound of value that can get with the left capacity
	// taken:         current take/no-take choice of each item
	// pos:           next item to consider

	auto start_value = 0.0;
	auto start_capacity = capacity;
	auto start_expectation = get_expectation(items, capacity, 0);
	auto start_taken = vector<int>(items.size(), 0);
	auto start_pos = 0;

	using StackElem = tuple<double, int, double, vector<int>, int>;
	vector<StackElem> stack;
	stack.push_back(make_tuple(start_value, start_capacity, start_expectation, start_taken, start_pos));
	//* initialize time measure
	long long index_search = 0;
	double expected_time = 10.0;
	clock_t start_time = clock();
	clock_t current_time, end_time = start_time + expected_time * CLOCKS_PER_SEC;

	while(!stack.empty())
	{
		auto [cur_value, cur_capacity, cur_expectation, cur_taken, cur_pos] = stack.back();
		stack.pop_back();

		// if left capacity is not enough, then backtrack
		if(cur_capacity < 0) continue;
		
		// if current expectation is smaller than the best value, then backtrack
		if(cur_expectation <= max_value) continue;

		// if max value is smaller than current value, update max value and its item-take choices
		if(max_value < cur_value)
		{
			max_value = cur_value;
			max_taken = cur_taken;
		}

		// if next item to consider dose not exist, then backtrack
		if(cur_pos >= items.size()) continue;

		auto cur_item = items[cur_pos];
    
		// try not to take the next item
        auto notake_value = cur_value;
        auto notake_capacity = cur_capacity;
        auto notake_expectation = notake_value + get_expectation(items, notake_capacity, cur_pos + 1);
        auto notake_taken = cur_taken;
        
        stack.push_back(make_tuple(notake_value, notake_capacity, notake_expectation, notake_taken, cur_pos + 1));
    
		// try to take the next item
        auto take_value = cur_value + cur_item.value;
        auto take_capacity = cur_capacity - cur_item.weight;
        auto take_expectation = take_value + get_expectation(items, take_capacity, cur_pos + 1);
        auto take_taken = cur_taken;
        take_taken[cur_item.index] = 1;
        
        stack.push_back(make_tuple(take_value, take_capacity, take_expectation, take_taken, cur_pos + 1));

		index_search++;
		if(index_search % 100 == 0){
			if((current_time = clock()) >= end_time) break;
		}
	}
	return make_tuple(static_cast<int>(max_value), max_taken);
}

// print the content of a vector
auto print_vec(const vector<int> & vec, FILE * f = stdout)
{
	for(auto i = 0; i < vec.size(); ++i)
	{
		fprintf(f, "%d", vec[i]);
		if(i + 1 == vec.size()) fprintf(f, "\n");
		else fprintf(f, " ");
	}
}

auto load_item(const char * filename)
{
	auto f = fopen(filename, "r");
	assert(f);

	int item_count, capacity;
	fscanf(f, "%d %d", &item_count, &capacity);
	vector<Item> items;
	for(auto i = 0; i < item_count; ++i)
	{
		int value;
		int weight;
		fscanf(f, "%d %d", &value, &weight);

		items.push_back(Item{ i, value, weight });
	}

	fclose(f);

	return make_tuple(items, capacity);
}

auto save_item(const char * filename, int value, const vector<int> & taken)
{
	// write result to cpp_output.txt, so that solver.py can read result from it
	auto f = fopen(filename, "w");
	assert(f);

	fprintf(f, "%d 1\n", value);
	print_vec(taken, f);
	fclose(f);
}

int main(int argc, char * argv[]){
    if(argc < 2)
    {
        printf("Usage: ./main <data-file>\n");
        printf("Example: ./main data/ks_30_0\n");
        exit(-1);
    }
    //* load knapsack candidates from data file
	auto [items, capacity] = load_item(argv[1]);
    //* solve knapsack
    //auto [value, taken] = solveByDp(items, capacity);
    sort(items.begin(), items.end());
    auto [value, taken] = search(items, capacity);

    //* print result
    printf("%d 1\n", value);
	print_vec(taken);
    //* save result to file
	//save_item("cpp_output.txt", value, taken);
    return 0;
}
