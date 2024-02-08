/*--------------------------------------------
   Program 6: File Compression
   For this project, I will build a file compression algorithm that uses binary trees and priority queues. The program is built on the Huffman algorithm to allow the user to compress or decompress a file.
   Course: CS 251, Fall 2022
   System: Mac/Windows using VS Code
   Author: Sebastian Kowalczyk
---------------------------------------------*/

#pragma once

typedef hashmap hashmapF;
typedef unordered_map<int, string> hashmapE;

struct HuffmanNode
{
    int character;
    int count;
    HuffmanNode *zero;
    HuffmanNode *one;
};

struct compare
{
    bool operator()(const HuffmanNode *lhs,
                    const HuffmanNode *rhs)
    {
        return lhs->count > rhs->count;
    }
};

//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode *node)
{
    if (node == NULL)
    {
        return;
    }
    freeTree(node->zero);
    freeTree(node->one);
    delete node;
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmapF &map)
{
    char c;
    ifstream inFS(filename);
    if (isFile)
    {
        while (inFS.get(c))
        {
            int key = (int)c;
            // check if map already has key
            if (map.containsKey(key))
            {
                int value = map.get(key); // get value
                map.put(key, value + 1);  // increment value
            }
            else
            {
                map.put(key, 1); // add key and value if not in map
            }
        }
        // add an EOF character
        map.put(PSEUDO_EOF, 1);
    }
    else
    { // same logic applies except this is for a string rather than a file
        for (char c : filename)
        {
            int key = (int)c;
            // check if map already has key
            if (map.containsKey(key))
            {
                int value = map.get(key);
                map.put(key, value + 1);
            }
            else
            {
                map.put(key, 1);
            }
        }
    }
}

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode *buildEncodingTree(hashmapF &map)
{
    // intialize priority queue
    priority_queue<HuffmanNode *, vector<HuffmanNode *>, compare> pq;
    // add all nodes to priority queue
    // for every key value pair in the map create a new node then insert into priority queue
    // frequency is count, key is character
    for (int key : map.keys())
    {
        HuffmanNode *node = new HuffmanNode;
        node->character = key;
        node->count = map.get(key);
        node->zero = nullptr;
        node->one = nullptr;
        pq.push(node);
    }
    // grab two nodes from the queue and create a new node with the sum of the two counts
    // link them up to become binary tree
    // enqueue the new node back into the queue
    // repeat until only one node is left
    while (pq.size() > 1)
    {
        HuffmanNode *first = pq.top();
        pq.pop();
        HuffmanNode *second = pq.top();
        pq.pop();
        HuffmanNode *newNode = new HuffmanNode;
        newNode->character = NOT_A_CHAR;
        newNode->count = first->count + second->count;
        newNode->zero = first;
        newNode->one = second;
        pq.push(newNode);
    }

    return pq.top();
}

//
// *Recursive helper function for building the encoding map.
//
void _buildEncodingMap(HuffmanNode *node, hashmapE &encodingMap, string str,
                       HuffmanNode *prev)
{

    if (node->character != NOT_A_CHAR)
    {                                       // if node is a leaf
        encodingMap[node->character] = str; // add to encoding map
        return;
    }
    if (node->zero != nullptr)
    {                                                                // if node has a left child
        _buildEncodingMap(node->zero, encodingMap, str + "0", node); // add 0 to string and recurse
    }
    if (node->one != nullptr)
    {                                                               // if node has a right child
        _buildEncodingMap(node->one, encodingMap, str + "1", node); // add 1 to string and recurse
    }
}

//
// *This function builds the encoding map from an encoding tree.
//
hashmapE buildEncodingMap(HuffmanNode *tree)
{
    hashmapE encodingMap;
    _buildEncodingMap(tree, encodingMap, "", nullptr); // call recursive helper function

    return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream &input, hashmapE &encodingMap, ofbitstream &output,
              int &size, bool makeFile)
{
    string result = "";
    char c;
    if (makeFile)
    {
        while (input.get(c))
        {
            int key = (int)c;
            string temp = encodingMap[key]; // temp storing value associated with key
            for (char c : temp)
            { // loop through val
                if (c == '0')
                {
                    output.writeBit(0); // write 0 to output stream
                    result += "0";
                }
                else
                {
                    output.writeBit(1);
                    result += "1"; // concatenate
                }
            }
        }
        // single occurance of pseudo eof, same logic applies as before
        string temp = encodingMap[PSEUDO_EOF];
        for (char c : temp)
        {
            if (c == '0')
            {
                output.writeBit(0);
                result += "0";
            }
            else
            {
                output.writeBit(1);
                result += "1";
            }
        }
    }
    // else makefile is false and don't write to the output file, just return string
    else
    {
        while (input.get(c))
        {
            int key = (int)c;
            string temp = encodingMap[key]; // temp storing value associated with key
            for (char c : temp)
            { // loop through val
                if (c == '0')
                {
                    result += "0";
                }
                else
                {
                    result += "1"; // concatenate
                }
            }
        }
        // single occurance of pseudo eof, same logic applies as before
        string temp = encodingMap[PSEUDO_EOF];
        for (char c : temp)
        {
            if (c == '0')
            {
                result += "0";
            }
            else
            {
                result += "1";
            }
        }
    }

    size = result.length(); // size of result string
    return result;
}

//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode *encodingTree, ofstream &output)
{

    // for each bit in the input stream traverse the tree
    // if bit is 1 go right, if bit is 0 go left
    // if you reach a leaf add char to the output string and reset the root
    // go until you reach the pseudo eof

    string result = "";
    HuffmanNode *root = encodingTree;

    while (!input.eof())
    {
        int bit = input.readBit();
        if (bit == 0)
        {
            root = root->zero; // go left if bit 0
        }
        else
        {
            root = root->one; // go right if bit 1
        }
        if (root->character != NOT_A_CHAR)
        {
            if (root->character == PSEUDO_EOF)
            { // if eof is reached break out of the loop
                break;
            }

            result += (char)root->character; // add char to result string
            root = encodingTree;             // reset root
        }
    }
    /// put all characters from result into the output stream
    for (int i = 0; i < result.length(); i++)
    {
        output.put(result[i]);
    }

    return result;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename)
{
    // build frequency map
    hashmapF freqMap;
    buildFrequencyMap(filename, true, freqMap);
    // build encoding tree
    HuffmanNode *encodingTree = buildEncodingTree(freqMap);
    // build encoding map
    hashmapE encodingMap = buildEncodingMap(encodingTree);
    // prepare an output stream
    ofbitstream output(filename + ".huf");
    // output frequency map to ofbitstream
    output << freqMap;
    // prepare an input stream from the file
    ifstream input(filename);
    // encode the file
    int size;
    string result = encode(input, encodingMap, output, size, true);
    // free allocated memory
    freeTree(encodingTree);

    return result;
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename)
{
    // prepare an input stream from the huf file
    ifbitstream input(filename);
    // parse the filename to not include huf and instead include unc.txt
    string editedFile = filename.substr(0, filename.length() - 8);
    editedFile += "_unc.txt";
    // prepare an output stream
    ofstream output(editedFile);
    // build frequency map
    hashmapF freqMap;
    input >> freqMap;
    // build encoding tree
    HuffmanNode *encodingTree = buildEncodingTree(freqMap);
    // decode the file
    string result = decode(input, encodingTree, output);
    // free allocated memory
    freeTree(encodingTree);

    return result;
}
