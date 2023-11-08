#include "blockchain.h"




Vote::Vote(json blockchain) {
    Block_t bl = get_last(blockchain);
    Block_t block = vote(bl, get_vote());
    printf("block done:\n%s\n", block_to_string(block).c_str());
    std::pair<EVP_PKEY*, EVP_PKEY*> k =generate_keys(100);

}
Vote::Vote(){

}
Block_t Vote::vote(Block_t last_block, uint8_t res){
    Block_t block;

    block.header.prev_hash = last_block.header.hash;
    block.header.timestamp = get_timestamp();
    block.result = res;

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

    std::pair<EVP_MD_CTX*, EVP_PKEY_CTX*> ctx = init_ctx(sKey, SIGN);
    EVP_DigestSignInit(ctx.first, &ctx.second, EVP_sha256(), NULL, sKey);
    EVP_DigestSign(ctx.first, sign, len, (const unsigned char*)block.c_str(), block.length());

    EVP_MD_CTX_free(ctx.first);

}

std::pair<EVP_MD_CTX*, EVP_PKEY_CTX*> Vote::init_ctx(EVP_PKEY* key, int type){
    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    EVP_PKEY_CTX* pkey_ctx = EVP_PKEY_CTX_new(key, NULL);

    EVP_PKEY_sign_init(pkey_ctx);

    EVP_PKEY_CTX_set_signature_md(pkey_ctx, EVP_sha256());
    return std::make_pair(md_ctx, pkey_ctx);
}

bool Vote::verify(std::string block, unsigned char* sign, size_t len, EVP_PKEY* pKey){
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    Net::log_error(EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pKey));
    Net::log_error(EVP_DigestVerifyUpdate(ctx, block.c_str(), block.length()));
    int res = EVP_DigestVerifyFinal(ctx, sign, len);

    EVP_MD_CTX_free(ctx);
    return res;

}


