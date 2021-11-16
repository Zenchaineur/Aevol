//
// Created by arrouan on 01/10/18.
//

#include "Dna.h"

#include <cassert>

Dna::Dna(int length, Threefry::Gen &&rng) : seq_(length) {
    // Generate a random genome
    for (int32_t i = length; i > 0; i--) {
        seq_[i] = rng.random(NB_BASE);
    }
}

int Dna::length() const {
    return seq_.size();
}

void Dna::save(gzFile backup_file) {
    int dna_length = length();
    std::vector<char> seq_char;
    for(int i = 0; i < seq_.size(); i++){
        char c = seq_[seq_len - i - 1] + '0';
        seq_char.push_back(c);
    }
    gzwrite(backup_file, &dna_length, sizeof(dna_length));
    gzwrite(backup_file, seq_char.data(), dna_length * sizeof(seq_char[0]));
}

void Dna::load(gzFile backup_file) {
    int dna_length;
    gzread(backup_file, &dna_length, sizeof(dna_length));
    char tmp_seq[dna_length];
    gzread(backup_file, tmp_seq, dna_length * sizeof(tmp_seq[0]));
    std::vector<char> seq_char = std::vector<char>(tmp_seq, tmp_seq + dna_length);
    for(int i = 0; i < seq_char.size(); i++){
        seq_[seq_len - i - 1] = bool(seq_char[i] - '0');
    }
}

void Dna::do_switch(int pos) {
    seq_.flip(length() - pos - 1);
    // if (seq_[pos] == '0') seq_[pos] = '1';
    // else seq_[pos] = '0';
}

int Dna::promoter_at(int pos) {
    std::bitset<PROM_SIZE> prom_dist_bit;
    for (int motif_id = 0; motif_id < PROM_SIZE; motif_id++) {
        int search_pos = pos + motif_id;
        if (search_pos >= seq_.size())
            search_pos -= seq_.size();
        // Searching for the promoter
        prom_dist_bit[PROM_SIZE - motif_id - 1] = seq_[length() - search_pos - 1];
    }

    int dist_lead = (prom_dist_bit ^ prom_bitset).count();
    return dist_lead;
}

// Given a, b, c, d boolean variable and X random boolean variable,
// a terminator look like : a b c d X X !d !c !b !a
int Dna::terminator_at(int pos) {
    // store value of length() in variable _length before the loop
    const int _length = length();
    int term_dist[TERM_STEM_SIZE];
    for (int motif_id = 0; motif_id < TERM_STEM_SIZE; motif_id++) {
        int right = pos + motif_id;
        int left = pos + (TERM_SIZE - 1) - motif_id;

        // loop back the dna inf needed
        if (right >= _length) right -= _length;
        if (left >= _length) left -= _length;

        // Search for the terminators
        // term_dist[motif_id] = seq_[length() - right - 1] != seq_[length() - left - 1] ? 1 : 0;
        term_dist[motif_id] = seq_[length() - right - 1] != seq_[length() - left - 1];
    }
    int dist_term_lead = term_dist[0] +
                         term_dist[1] +
                         term_dist[2] +
                         term_dist[3];

    return dist_term_lead;
}

bool Dna::shine_dal_start(int pos) {
    bool start = false;
    int t_pos, k_t;

    for (int k = 0; k < SHINE_DAL_SIZE + CODON_SIZE; k++) {
        k_t = k >= SHINE_DAL_SIZE ? k + SD_START_SPACER : k;
        t_pos = pos + k_t;
        if (t_pos >= seq_.size())
            t_pos -= seq_.size();
        if (seq_[length() - t_pos - 1] == shine_dal_bitset[SD_TO_START - k_t - 1]) {
            start = true;
        } else {
            start = false;
            break;
        }
    }

    return start;
}

bool Dna::protein_stop(int pos) {
    bool is_protein;
    int t_k;

    for (int k = 0; k < CODON_SIZE; k++) {
        t_k = pos + k;
        if (t_k >= seq_.size())
            t_k -= seq_.size();

        if (seq_[length() - t_k - 1] == protein_end_bitset[PROTEIN_END_SIZE - k - 1]) {
            is_protein = true;
        } else {
            is_protein = false;
            break;
        }
    }

    return is_protein;
}

int Dna::codon_at(int pos) {
    int value = 0;

    int t_pos;

    for (int i = 0; i < CODON_SIZE; i++) {
        t_pos = pos + i;
        if (t_pos >= seq_.size())
            t_pos -= seq_.size();
        if (seq_[length() - t_pos - 1] == true)
            value += 1 << (CODON_SIZE - i - 1);
    }

    return value;
}