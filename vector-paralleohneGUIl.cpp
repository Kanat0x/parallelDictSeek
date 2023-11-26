// C++ program to demonstrate auto-complete feature 
// using Trie data structure. 
#include <algorithm>

#include <iomanip>

#include <iostream>
#include <vector>
#include <windows.h>
#include <chrono>
#include <thread>
#include <future>
using namespace std;

// Alphabet size (# of symbols) 
#define ALPHABET_SIZE (26) 

// Converts key current character into index 
// use only 'a' through 'z' and lower case 
//#define CHAR_TO_INDEX(c) ((int)c - (int)'a') 

// Converts key current character into index 
// use only 'a' through 'z' and lower case 
#define CHAR_TO_INDEX(c) ((int)toupper(c) - (int)'A')

// trie node 
struct TrieNode
{
	struct TrieNode* children[ALPHABET_SIZE];

	// isWordEnd is true if the node represents 
	// end of a word 
	bool isWordEnd;
};

// Returns new trie node (initialized to NULLs) 
struct TrieNode* getNode(void)
{
	struct TrieNode* pNode = new TrieNode;
	pNode->isWordEnd = false;

	for (int i = 0; i < ALPHABET_SIZE; i++)
		pNode->children[i] = NULL;

	return pNode;
}

// If not present, inserts key into trie. If the 
// key is prefix of trie node, just marks leaf node 
void insert(struct TrieNode* root, const string key)
{
	struct TrieNode* pCrawl = root;

	for (int level = 0; level < key.length(); level++)
	{
		int index = CHAR_TO_INDEX(key[level]);
		if (!pCrawl->children[index])
			pCrawl->children[index] = getNode();

		pCrawl = pCrawl->children[index];
	}

	// mark last node as leaf 
	pCrawl->isWordEnd = true;
}

// Returns true if key presents in trie, else false 
bool search(struct TrieNode* root, const string key)
{
	int length = key.length();
	struct TrieNode* pCrawl = root;
	for (int level = 0; level < length; level++)
	{
		int index = CHAR_TO_INDEX(key[level]);

		if (!pCrawl->children[index])
			return false;

		pCrawl = pCrawl->children[index];
	}

	return (pCrawl != NULL && pCrawl->isWordEnd);
}

// Returns 0 if current node has a child 
// If all children are NULL, return 1. 
bool isLastNode(struct TrieNode* root)
{
	for (int i = 0; i < ALPHABET_SIZE; i++)
		if (root->children[i])
			return 0;
	return 1;
}

// Recursive function to print auto-suggestions for given 
// node. 
void suggestionsRec(struct TrieNode* root, string currPrefix)
{
    // found a string in Trie with the given prefix 
    if (root->isWordEnd)
    {
        cout << currPrefix;
        cout << endl;
    }

    // All children struct node pointers are NULL 
    if (isLastNode(root))
        return;

    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        if (root->children[i])
        {
            // append current character to currPrefix string 
            currPrefix.push_back(65 + i);  // Convert back to uppercase

            // recur over the rest 
            suggestionsRec(root->children[i], currPrefix);

            // Remove the last character for the next iteration
            currPrefix.pop_back();
        }
    }
}

// print suggestions for given query prefix. 
int printAutoSuggestions(TrieNode* root, const string query)
{
	struct TrieNode* pCrawl = root;

	// Check if prefix is present and find the 
	// the node (of last level) with last character 
	// of given string. 
	int level;
	int n = query.length();
	for (level = 0; level < n; level++)
	{
		int index = CHAR_TO_INDEX(query[level]);

		// no string in the Trie has this prefix 
		if (!pCrawl->children[index])
			return 0;

		pCrawl = pCrawl->children[index];
	}

	// If prefix is present as a word. 
	bool isWord = (pCrawl->isWordEnd == true);

	// If prefix is last node of tree (has no 
	// children) 
	bool isLast = isLastNode(pCrawl);

	// If prefix is present as a word, but 
	// there is no subtree below the last 
	// matching node. 
	if (isWord && isLast)
	{
		cout << query << endl;
		return -1;
	}

	// If there are are nodes below last 
	// matching character. 
	if (!isLast)
	{
		string prefix = query;
		suggestionsRec(pCrawl, prefix);
		return 1;
	}
}


// Function to parallelize trie building
void buildTrieParallel(struct TrieNode* root, const vector<string>& words, size_t start, size_t end)
{
    for (size_t i = start; i < end; ++i)
    {
        insert(root, words[i]);
    }
}

// Function to parallelize trie search
int searchTrieParallel(TrieNode* root, const vector<string>& queries, size_t start, size_t end)
{
    int result = 0;
    for (size_t i = start; i < end; ++i)
    {
        result = printAutoSuggestions(root, queries[i]);
    }
    return result;
}


// Driver Code 
int main()
{


    vector<string> words;
    for (char a = 'A'; a <= 'Z'; ++a) {
        for (char b = 'A'; b <= 'Z'; ++b) {
            for (char c = 'A'; c <= 'Z'; ++c) {
                for (char d = 'A'; d <= 'Z'; ++d) {
                    words.push_back(string({ a, b, c, d}));
                }
            }
        }   
    }


    const size_t numThreads = thread::hardware_concurrency();  // Get the number of hardware threads
    const size_t wordsPerThread = words.size() / 4;

    auto startBuild = std::chrono::high_resolution_clock::now();

	struct TrieNode* root = getNode();
    
    //Ohne threads
	// Fügen Sie jedes Wort aus dem Vektor in die TRIE ein
	/*
    for (const auto& word : words) {
        insert(root, word);
    }*/

    
   // mit threads
    vector<future<void>> futures;
    for (size_t i = 0; i < 4; ++i)
    {
        size_t start = i * wordsPerThread;
        size_t end = (i == 4 - 1) ? words.size() : start + wordsPerThread;
        futures.push_back(async(launch::async, buildTrieParallel, root, ref(words), start, end));
    }

    // wait for threads to finish
    for (auto& fut : futures)
    {
        fut.get();
    }

    auto stopBuild = std::chrono::high_resolution_clock::now();

    /*string prefix;
    cout << "Please insert prefix to search for: ";
	cin >> prefix;*/

	string prefix = "";




    
	auto startSearch = std::chrono::high_resolution_clock::now();
	int comp = printAutoSuggestions(root, prefix);
    auto stopSearch = std::chrono::high_resolution_clock::now();

    auto buildDuration = std::chrono::duration<double, std::milli>(stopBuild - startBuild);
    auto searchDuration = std::chrono::duration<double, std::milli>(stopSearch - startSearch);
    cout << "Trie data structure built in: " << std::fixed << std::setprecision(5) << buildDuration.count() << " milliseconds" << endl;
    cout << "Search completed in: " << std::fixed << std::setprecision(5) << searchDuration.count() << " milliseconds" << endl;
    
	if (comp == 0)
		cout << "No Strings with prefix: " << prefix << endl;

	return 0;
}
