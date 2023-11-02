#include "blockchain.h"

Vote::Vote(json blockchain, uint8_t res) {
    Block_t bl = get_last(blockchain);
    Block_t block = vote(bl, res);
    printf("block done:\n%s\n", block_to_string(block).c_str());
    std::pair<EVP_PKEY*, EVP_PKEY*> k =generate_keys(100);

}
Vote::Vote(){

}
Block_t Vote::vote(Block_t last_block, uint8_t res){
    Block_t block = init_block(last_block.header.hash, res); 
    return block;
}


std::pair<EVP_PKEY*, EVP_PKEY*> Vote::generate_keys(int length){
    EVP_PKEY_CTX *ctx;
    EVP_PKEY *sKey = NULL, *pKey = NULL;

    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL); 
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, length);
    EVP_PKEY_keygen(ctx, &sKey);

    pKey = EVP_PKEY_dup(sKey);
    EVP_PKEY_CTX_free(ctx);

    return std::make_pair(sKey, pKey);
}

void Vote::get_signature(EVP_PKEY* sKey, std::string block, unsigned char* sign, size_t* len){

    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    EVP_PKEY_CTX* pkey_ctx = EVP_PKEY_CTX_new(sKey, NULL);
    EVP_PKEY_sign_init(pkey_ctx);
    EVP_PKEY_CTX_set_signature_md(pkey_ctx, EVP_sha256());
    EVP_DigestSignInit(md_ctx, &pkey_ctx, EVP_sha256(), NULL, sKey);
    
    EVP_DigestSign(md_ctx, sign, len, (const unsigned char*)block.c_str(), block.length());

    EVP_MD_CTX_free(md_ctx);

}
bool Vote::verify(Block_t block, std::string signature, std::string pKey){
    //TODO
    return 0;

}
