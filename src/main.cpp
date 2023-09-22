#include "block.h"

int main () {
    Block block(2);
    std::vector<Block_t> blockchain;
    Block_t b = block.init_block("0000", 0);
    blockchain.push_back(b);
    Block_t b2 = block.init_block(b.header.hash, 1);
    block.link_block(&blockchain, b2);

    return 0;
}
