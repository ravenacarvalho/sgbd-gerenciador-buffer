#include <iostream>
#include "buffer.h" 

void Evict(int politica, std::vector<Page>& buffer, int& clock_pointer) {
    if (buffer.empty()) return; 
    int indice_removido = 0;
    if (politica == POLITICA_FIFO) {
        indice_removido = 0; 
    } 
    else if (politica == POLITICA_LRU) {
        int min_time = buffer[0].last_access_time;
        for (size_t i = 1; i < buffer.size(); ++i) {
            if (buffer[i].last_access_time < min_time) {
                min_time = buffer[i].last_access_time;
                indice_removido = i;
            }
        }
    } 
    else if (politica == POLITICA_CLOCK) {
       while (true) {
            clock_pointer = clock_pointer % buffer.size(); 

            if (buffer[clock_pointer].reference_bit == true) {
                buffer[clock_pointer].reference_bit = false;
                clock_pointer++;
            } else {
                indice_removido = clock_pointer;
                break;
            }
        }
    } 
    else if (politica == POLITICA_MRU) {
        int max_time = buffer[0].last_access_time;
        for (size_t i = 1; i < buffer.size(); ++i) {
            if (buffer[i].last_access_time > max_time) {
                max_time = buffer[i].last_access_time;
                indice_removido = i;
            }
        }
    } 
    std::cout << buffer[indice_removido].content;
    
    if (buffer[indice_removido].updated == true) {
        std::cout << " W"; 
    }
    std::cout << "\n";
    buffer.erase(buffer.begin() + indice_removido);
    if (politica == POLITICA_CLOCK && buffer.size() > 0) {
        clock_pointer = clock_pointer % buffer.size();
    } else if (buffer.size() == 0) {
        clock_pointer = 0;
    }
}