#include "blockchain.h"

Vote::Vote(json blockchain, uint8_t res) {
    Block_t bl = get_last(blockchain);
    Block_t block = vote(bl, res);
    printf("block done:\n%s\n", block_to_string(block).c_str());
}
Block_t Vote::vote(Block_t last_block, uint8_t res){
    Block_t block = init_block(last_block.header.hash, res); 
    return block;
}
