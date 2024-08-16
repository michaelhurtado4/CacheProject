#include <iostream>
#include <vector>
#include <cstdio>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>

// Block class that contains if the block is valid and the tag with it
class Block {
private:
    bool valid;
    unsigned long long int tag;

public:
    
    // Constructor, Invalid on creation
    Block() {
        valid = false;
    }

    // Constructor, Valid on creation
    Block(unsigned long long int Newtag) {
        valid = true;
        tag = Newtag;
    }

    // Destructor
    ~Block() {
        
    }

    // Getter for block tag
    unsigned long long int getBlockTag() const {
        return tag;
    }

    // Getter for block validity
    bool isValid() const {
        return valid;
    }

};

void Operation(unsigned long long int address, std::vector<std::vector<Block*>>& cache, int blocksize, unsigned long long int sets, int assoc, int& misses, char repl) {
    
    // calculate the number of bits for offset and set
    int offsetBits = log2(blocksize);
    int setBits = log2(sets);

    // shift away the offset bits
    address = address >> offsetBits;

    // get the set from the address remaining
    unsigned long long mask = (1ULL << setBits) - 1;
    unsigned long long set = address & mask;

    // shift away the set bits and the result equal to the tag
    unsigned long long tag = address >> setBits;
    
    // get set that the address wants
    std::vector<Block*>& checkSet = cache[set];

    // check if tag matches with a block in set 
    for (int i = 0; i < assoc; ++i) {
        if ((checkSet[i]->isValid() == true) && (checkSet[i]->getBlockTag() == tag)) {

            // put recently read at the front of the set
            Block* temp = new Block(tag);
            checkSet.erase(checkSet.begin() + i);
            checkSet.insert(checkSet.begin(), temp);
            return;
        }
    }

    // if set isn't full then just put new block at front 
    if (checkSet.size() < assoc) {
        Block* temp = new Block(tag);
        checkSet.insert(checkSet.begin(), temp);
    }
    // if set full then delete last used block and insert new tag at front
    else {
        if (repl == 'l') {
            delete checkSet.back();
            Block* temp = new Block(tag);
            checkSet.pop_back();
            checkSet.insert(checkSet.begin(), temp);
        }
        else {
            int deleteIndex = std::rand() % assoc;
            delete checkSet[deleteIndex]; 
            Block* temp = new Block(tag);
            checkSet[deleteIndex] = temp;
        }   
    }
    // increment miss by one because wasn't found
    misses += 1;
}


int main(int argc, char* argv[]) {

    // seed the random number 
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // get each variable from command line 
    int nk = std::stoi(argv[1]);
    int assoc = std::stoi(argv[2]);
    int blocksize = std::stoi(argv[3]);
    // if blocksize isn't a power of 2 make it a power of 2 
    blocksize = pow(2, ceil(log2(blocksize)));
    char repl = *(argv[4]);

    // calculate number of sets and make it a power of 2 if it isn't
    unsigned long long int sets = (nk * 1000) / (assoc * blocksize);
    sets = pow(2, ceil(log2(sets)));

    // initalize 2d vector of blocks with sets as rows and columns as associativity
    std::vector<std::vector<Block*>> cache(sets);
    for (unsigned long long int i = 0; i < sets; ++i) {
        cache[i] = std::vector<Block*>(assoc);
    }
    // Default each element in the vector with an invalid block
    for (unsigned long long int i = 0; i < sets; ++i) {
        for (int j = 0; j < assoc; ++j) {
            cache[i][j] = new Block();
        }
    }

    // initialize variables to track for output
    std::string line;
    int readChecks = 0;
    int writeChecks = 0;
    int readMisses = 0;
    int writeMisses = 0;

    // get each line from the trace file
    while (std::getline(std::cin, line)) {

        // set the operation and address from the specific line
        char operation;
        unsigned long long int address;
        std::istringstream iss(line);
        iss >> operation >> std::hex >> address;

        // process the operation if read or write, and if LRU or random

        // Read operation
        if (operation == 'r') {
            readChecks += 1;
            
            // Do LRU or random operation based on repl
            Operation(address, cache, blocksize, sets, assoc, readMisses, repl);

        } 
        // Write operation
        else if (operation == 'w') {
            writeChecks += 1;
            
            // Do LRU or random operation based on repl
            Operation(address, cache, blocksize, sets, assoc, writeMisses, repl);
        }
    }

    // iniialize and calculate variables for output
    int totalChecks = readChecks + writeChecks;
    int totalMisses = readMisses + writeMisses;
    double percentTotal = ((double)totalMisses / (double)totalChecks) * 100;
    double percentWrite = ((double)writeMisses / (double)writeChecks) * 100;
    double percentRead = ((double)readMisses / (double)readChecks) * 100;

    // output results 
    std::cout << std::fixed << std::setprecision(6) << std::dec << totalMisses << " " << percentTotal << "% " << readMisses << " " << percentRead << "% " << writeMisses << " " << percentWrite << "%" << std::endl;
}