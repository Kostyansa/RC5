//
//  main.cpp
//  RC5
//
//  Created by Constantine on 29.10.2020.
//  Copyright © 2020 Kostyansa. All rights reserved.
//

#include <iostream>
#include <vector>
#include <cmath>

#define debug
#ifdef debug
#include <sstream>
#endif

typedef unsigned long long ull;

typedef unsigned short int word;

const double e = exp(1);
const double phi = (1 + sqrt(5)) / 2;

template<class T>
T cycleLeft(T x, unsigned int shift)
{
    shift %= sizeof(T)*CHAR_BIT;
    return (x << shift) | (x >> (sizeof(T)*CHAR_BIT - shift));
}

template<class T>
T cycleRight(T x, unsigned int shift)
{
    shift %= sizeof(T)*CHAR_BIT;
    return (x >> shift) | (x << (sizeof(T)*CHAR_BIT - shift));
}

std::pair<word, word> generateMagicNumbers(const unsigned int& width){
    word P;
    word Q;
    P = (word) ((e - 2) * ((ull) 1 << width));
    P += (1 - P % 2);
    Q = (word) ((phi - 2) * ((ull) 1 << width));
    Q -= (1 - Q % 2);
    return std::make_pair(P, Q);
}

std::vector<word> initializeSubkeys(
                                    const std::pair<ull, ull>& magic_numbers,
                                    const unsigned int & rounds
                                    ){
    std::vector<word> subkeys(2 * (rounds + 1));
    subkeys[0] = magic_numbers.first;
    for (long i = 1; i < subkeys.size(); i++) {
        subkeys[i] = subkeys[i - 1] + magic_numbers.second;
    }
    return subkeys;
}

std::vector<word> mixSubkeys(const std::vector<word>& subkeys, const std::vector<word>& key){
    word A = 0, B = 0;
    std::vector<word> s = subkeys;
    std::vector<word> k = key;
    for (long z = 0, i = 0, j = 0;
         z < 3*std::max(s.size(), k.size());
         z++,
         i = (i + 1) % s.size() ,
         j = (j + 1) % k.size()) {
        A = s[i] = cycleLeft((s[i] + A + B), 3);
        B = k[j] = cycleLeft((k[j] + A + B), A + B);
    }
    return s;
}

std::pair<word, word> encryptBlock(word left, word right, const std::vector<word>& subkeys){
    left += subkeys[0];
    right += subkeys[1];
    for (long i = 1; i < subkeys.size() / 2; i++) {
        left ^= right;
        left = cycleLeft(left, right);
        left += subkeys[2 * i];
        right ^= left;
        right = cycleLeft(right, left);
        right += subkeys[2 * i + 1];
    }
    return std::make_pair(left, right);
}

std::pair<word, word> decryptBlock(word left, word right, const std::vector<word>& subkeys){
    for (long i = (subkeys.size() / 2) - 1; i >= 1; i--) {
        right -= subkeys[2 * i + 1];
        right = cycleRight(right, left);
        right ^= left;
        left -= subkeys[2 * i];
        left = cycleRight(left, right);
        left ^= right;
    }
    right -= subkeys[1];
    left -= subkeys[0];
    return std::make_pair(left, right);
}

std::vector<word> encrypt(
                          const std::vector<word>& message,
                          const std::vector<word>& key,
                          const unsigned int& width,
                          const unsigned int& rounds
                          ){
    std::vector<word> encrypted(message.size());
    std::pair<word, word> magic_numbers = generateMagicNumbers(width);
    
    std::vector<word> subkeys = initializeSubkeys(magic_numbers, rounds);
    subkeys = mixSubkeys(subkeys, key);
    
#ifdef debug_trace
    for (int i = 0; i < subkeys.size(); i++) {
        std::cout << subkeys[i] << " ";
    }
    std::cout << std::endl;
#endif
    
    for (long i = 0; i < message.size()/2; i++) {
        std::pair<word, word> encrypted_block = encryptBlock(message[2 * i],
                                                              message[2 * i + 1],
                                                              subkeys);
        encrypted[2 * i] = encrypted_block.first;
        encrypted[2 * i + 1] = encrypted_block.second;
    }
    return encrypted;
}

std::vector<word> decrypt(
                          const std::vector<word>& message,
                          const std::vector<word>& key,
                          const unsigned int& width,
                          const unsigned int& rounds
                          ){
    std::vector<word> decrypted(message.size());
    std::pair<word, word> magic_numbers = generateMagicNumbers(width);
    
    std::vector<word> subkeys = initializeSubkeys(magic_numbers, rounds);
    subkeys = mixSubkeys(subkeys, key);
    
    for (long i = 0; i < message.size()/2; i++) {
        std::pair<word, word> decrypted_block = decryptBlock(message[2 * i],
                                                              message[2 * i + 1],
                                                              subkeys);
        decrypted[2 * i] = decrypted_block.first;
        decrypted[2 * i + 1] = decrypted_block.second;
    }
    return decrypted;
}

std::vector<word> inputWords(const unsigned int& size){
    std::vector<word> words;
    for (long i = 0; i < size; i++) {
        word input;
        std::cin >> input;
        words.push_back(input);
    }
    return words;
}


#ifdef debug
void tests(){
    const unsigned int key_length = 8;
    const unsigned int width = 16;
    const unsigned int rounds = 255;
    
    std::vector<word> message = {
        1,
        65535,
        1,
        65535
    };
    std::vector<word> key = {
        65535,
        65535,
        65535,
        65535
    };
    
    std::cout << "Cycle test: " << (bool) (cycleLeft((word) 9, 64) == 9) << std::endl;
    
    std::vector<word> encrypted = encrypt(message, key, width, rounds);
    std::vector<word> decrypted = decrypt(encrypted, key, width, rounds);
    
    for (int i = 0; i < encrypted.size(); i++) {
        std::cout << message[i] << " " << encrypted[i] << " " << decrypted[i] << std::endl;
    }
    
    std::vector<word> subkeys = mixSubkeys(initializeSubkeys(generateMagicNumbers(width), rounds), key);
    std::pair<word, word> block = decryptBlock(encrypted[0], encrypted[1], subkeys);
    std::cout << "Block: " << block.first << " " << block.second << std::endl;
}
#endif

int main(int argc, const char * argv[]) {
    const unsigned int key_length = 8;
    const unsigned int width = 16;
    const unsigned int rounds = 16;
    
    std::cout << "Algorithm RC5-" << width/CHAR_BIT << "/" << rounds << "/" << key_length << std::endl;
    
    int n;
    bool finished = false;
#ifdef debug
    tests();
#endif
#ifndef debug
    do {
        std::cout << "Input 1 to Encrypt message" << std::endl;
        std::cout << "Input 2 to Decrypt message" << std::endl;
        std::cout << "Input 3 to exit" << std::endl;
        std::cin.clear();
        std::cin >> n;
        switch (n) {
            case 1:{
                unsigned int size;
                std::cout << "Input size of your message in blocks of " << width/CHAR_BIT << " bytes" << std::endl;
                std::cin.clear();
                std::cin >> size;
                std::cout << "Input your message in blocks of " << width/CHAR_BIT << " bytes divided by spaces" << std::endl;
                std::cin.clear();
                std::vector<word> message = inputWords(size);
                std::cout << "Input your key size " << key_length <<" bytes in blocks of " << width/CHAR_BIT << " bytes divided by spaces" << std::endl;
                std::cin.clear();
                std::vector<word> key = inputWords(key_lenght/(width/CHAR_BIT));
                std::vector<word> encrypted = encrypt(message, key, width, rounds);
                std::cout << "Your encrypted message: " << std::endl;
                for (int i = 0; i < encrypted.size(); i++) {
                    std::cout << encrypted[i] << " ";
                }
                std::cout << std::endl;
                break;
            }
            case 2:{
                unsigned int size;
                std::cout << "Input size of your message in blocks of " << width/CHAR_BIT << " bytes" << std::endl;
                std::cin.clear();
                std::cin >> size;
                std::cout << "Input your message in blocks of " << width/CHAR_BIT << " bytes divided by spaces" << std::endl;
                std::cin.clear();
                std::vector<word> message = inputWords(size);
                std::cout << "Input your key size " << key_length <<" in blocks of " << width/CHAR_BIT << " bytes divided by spaces" << std::endl;
                std::cin.clear();
                std::vector<word> key = inputWords(key_lenght/(width/CHAR_BIT));
                std::vector<word> decrypted = decrypt(message, key, width, rounds);
                std::cout << "Your decrypted message: " << std::endl;
                for (int i = 0; i < decrypted.size(); i++) {
                    std::cout << decrypted[i] << " ";
                }
                std::cout << std::endl;
                break;
            }
            case 3:{
                finished = true;
                break;
            }
            default:{
                std::cout << "Invalid command" << std::endl;
            }
        }
    } while (!finished);
#endif
    
    return 0;
}
