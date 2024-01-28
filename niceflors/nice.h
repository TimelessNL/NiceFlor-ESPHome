#include <stdint.h>
#include <stdio.h>

uint64_t nice_flor_s_encode(uint32_t serial, uint32_t code, uint8_t button_id, uint8_t repeat)
{
    uint8_t snbuff[4] = {0};
    snbuff[0] = serial & 0xff;
    snbuff[1] = (serial & 0xff00) >> 8;
    snbuff[2] = (serial & 0xff0000) >> 16;
    snbuff[3] = (serial & 0xff000000) >> 24;
        
    uint8_t  encbuff[7] = {0};
    uint32_t enccode    = pgm_read_word_near(nice_flor_s_table_decode + code);
    uint8_t  ki         = nice_flor_s_table_ki[code & 0xff] ^ (enccode & 0xff);
    
    encbuff[0] = button_id & 0x0f;
    encbuff[1] = ((repeat ^ button_id ^ 0x0f) << 4) | ((snbuff[3] ^ ki) & 0x0f);
    encbuff[2] = enccode >> 8;
    encbuff[3] = enccode & 0xff;
    encbuff[4] = snbuff[2] ^ ki;
    encbuff[5] = snbuff[1] ^ ki;
    encbuff[6] = snbuff[0] ^ ki;

    uint64_t encoded = 0;
    encoded |= ((uint64_t)(encbuff[6] << 4) & 0xF0) << 0;
    encoded |= ((uint64_t)((encbuff[5] & 0x0F) << 4) | ((uint64_t)(encbuff[6] & 0xF0) >> 4)) << 8;
    encoded |= ((uint64_t)((encbuff[4] & 0x0F) << 4) | ((uint64_t)(encbuff[5] & 0xF0) >> 4)) << 16;
    encoded |= ((uint64_t)((encbuff[3] & 0x0F) << 4) | ((uint64_t)(encbuff[4] & 0xF0) >> 4)) << 24;
    encoded |= ((uint64_t)(encbuff[2] & 0x0F) << 4 | (uint64_t)(encbuff[3] & 0xF0) >> 4) << 32;
    encoded |= ((uint64_t)(encbuff[1] & 0x0F) << 4 | (uint64_t)(encbuff[2] & 0xF0) >> 4) << 40;
    encoded |= ((uint64_t)(encbuff[0] & 0x0F) << 4 | (uint64_t)(encbuff[1] & 0xF0) >> 4) << 48;
    encoded = encoded ^ 0xFFFFFFFFFFFFF0;
    encoded = encoded >> 4;

    return encoded;
}

void nice_flor_s_tx_10(std::vector<int>* tx_buffer)
{
    tx_buffer->push_back(id(tx_short));
    tx_buffer->push_back(-id(tx_long));
}

void nice_flor_s_tx_11(std::vector<int>* tx_buffer)
{
    tx_buffer->push_back(id(tx_long));
    tx_buffer->push_back(-id(tx_short));
}

void nice_flor_s_tx_sync(std::vector<int>* tx_buffer)
{
    tx_buffer->push_back(id(tx_sync));
    tx_buffer->push_back(-id(tx_sync));
}

void nice_flor_s_tx_gap(std::vector<int>* tx_buffer)
{
    tx_buffer->push_back(-id(tx_gap));
}

std::vector<int> nice_flor_s_generate_tx(uint64_t data)
{
    std::vector<int> tx_buffer;

    nice_flor_s_tx_sync(&tx_buffer);

    for (int i = id(nb_bits) - 1; i >= 0; --i) {
        byte bit = (data >> i) & 1;
        if(bit){
           nice_flor_s_tx_10(&tx_buffer);
        }else{
           nice_flor_s_tx_11(&tx_buffer);
        }
    }

   nice_flor_s_tx_sync(&tx_buffer);
   nice_flor_s_tx_gap(&tx_buffer);
   return tx_buffer;
}
