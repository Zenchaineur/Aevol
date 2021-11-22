//
// Created by arrouan on 01/10/18.
//

#include "Dna.h"

#include <cassert>
#include <bitset>

Dna::Dna(int length, Threefry::Gen &&rng) : seq_(length) {
    // Generate a random genome
    for (int32_t i = 0; i < length; i++) {
        seq_[i] = '0' + rng.random(NB_BASE);
    }
}

int Dna::length() const {
    return seq_.size();
}

void Dna::save(gzFile backup_file) {
    int dna_length = length();
    gzwrite(backup_file, &dna_length, sizeof(dna_length));
    gzwrite(backup_file, seq_.data(), dna_length * sizeof(seq_[0]));
}

void Dna::load(gzFile backup_file) {
    int dna_length;
    gzread(backup_file, &dna_length, sizeof(dna_length));

    char tmp_seq[dna_length];
    gzread(backup_file, tmp_seq, dna_length * sizeof(tmp_seq[0]));

    seq_ = std::vector<char>(tmp_seq, tmp_seq + dna_length);
}

void Dna::set(int pos, char c) {
    seq_[pos] = c;
}

/**
 * Remove the DNA inbetween pos_1 and pos_2
 *
 * @param pos_1
 * @param pos_2
 */
void Dna::remove(int pos_1, int pos_2) {
    assert(pos_1 >= 0 && pos_2 >= pos_1 && pos_2 <= seq_.size());
    seq_.erase(seq_.begin() + pos_1, seq_.begin() + pos_2);
}

/**
 * Insert a sequence of a given length at a given position into the DNA of the Organism
 *
 * @param pos : where to insert the sequence
 * @param seq : the sequence itself
 * @param seq_length : the size of the sequence
 */
void Dna::insert(int pos, std::vector<char> seq) {
// Insert sequence 'seq' at position 'pos'
    assert(pos >= 0 && pos < seq_.size());

    seq_.insert(seq_.begin() + pos, seq.begin(), seq.end());
}

/**
 * Insert a sequence of a given length at a given position into the DNA of the Organism
 *
 * @param pos : where to insert the sequence
 * @param seq : the sequence itself
 * @param seq_length : the size of the sequence
 */
void Dna::insert(int pos, Dna *seq) {
// Insert sequence 'seq' at position 'pos'
    assert(pos >= 0 && pos < seq_.size());

    seq_.insert(seq_.begin() + pos, seq->seq_.begin(), seq->seq_.end());
}

void Dna::do_switch(int pos) {
    if (seq_[pos] == '0') seq_[pos] = '1';
    else seq_[pos] = '0';
}

void Dna::do_duplication(int pos_1, int pos_2, int pos_3) {
    // Duplicate segment [pos_1; pos_2[ and insert the duplicate before pos_3
    char *duplicate_segment = NULL;

    int32_t seg_length;

    if (pos_1 < pos_2) {
        //
        //       pos_1         pos_2                   -> 0-
        //         |             |                   -       -
        // 0--------------------------------->      -         -
        //         ===============                  -         - pos_1
        //           tmp (copy)                      -       -
        //                                             -----      |
        //                                             pos_2    <-'
        //
        std::vector<char> seq_dupl =
                std::vector<char>(seq_.begin() + pos_1, seq_.begin() + pos_2);

        insert(pos_3, seq_dupl);
    } else { // if (pos_1 >= pos_2)
        // The segment to duplicate includes the origin of replication.
        // The copying process will be done in two steps.
        //
        //                                            ,->
        //    pos_2                 pos_1            |      -> 0-
        //      |                     |                   -       - pos_2
        // 0--------------------------------->     pos_1 -         -
        // ======                     =======            -         -
        //  tmp2                        tmp1              -       -
        //                                                  -----
        //
        //
        std::vector<char>
                seq_dupl = std::vector<char>(seq_.begin() + pos_1, seq_.end());
        seq_dupl.insert(seq_dupl.end(), seq_.begin(), seq_.begin() + pos_2);

        insert(pos_3, seq_dupl);
    }
}

int Dna::promoter_at(int pos) {
    
    int dist_lead = 0;
    for (int motif_id = 0; motif_id < PROM_SIZE; motif_id++) {
        int search_pos = pos + motif_id;
        if (search_pos >= seq_.size())
            search_pos -= seq_.size();
        // Searching for the promoter
        //prom_dist.set(motif_id, PROM_SEQ[motif_id] != seq_[search_pos]);
        if(PROM_SEQ[motif_id] != seq_[search_pos]) dist_lead++;
    }

    return dist_lead;
}

// Given a, b, c, d boolean variable and X random boolean variable,
// a terminator look like : a b c d X X !d !c !b !a
int Dna::terminator_at(int pos) {
    // store value of length() in variable _length before the loop
    const int _length = length();
    int dist_term_lead = 0;
    for (int motif_id = 0; motif_id < TERM_STEM_SIZE; motif_id++) {
        int right = pos + motif_id;
        int left = pos + (TERM_SIZE - 1) - motif_id;

        // loop back the dna inf needed
        if (right >= _length) right -= _length;
        if (left >= _length) left -= _length;

        // Search for the terminators
        if(seq_[right] != seq_[left]) dist_term_lead++;
    }

    return dist_term_lead;
}

bool Dna::shine_dal_start(int pos) {
    int shine_start_dist[SHINE_DAL_SIZE];

    #pragma omp paralel for 
    for (int motif_id = 0; motif_id < SHINE_DAL_SIZE; motif_id++) {
        int search_pos = pos + motif_id;
        if (search_pos >= seq_.size()){
            search_pos -= seq_.size();
        }
        if((seq_[search_pos] != SHINE_DAL_SEQ[motif_id])&&((motif_id < 7)||(motif_id>10)))
        {
            return false;
        }
    }
    return true;
}

bool Dna::protein_stop(int pos) {
    int protein_stop_dist[3];

    #pragma omp paralel for 
    for (int motif_id = 0; motif_id < 3; motif_id++) {
        int search_pos = pos + motif_id;
        if (search_pos >= seq_.size()){
            search_pos -= seq_.size();
        }
        if(seq_[search_pos] != PROTEIN_END[motif_id]){
            return false;
        }
    }
    return true;
}

int Dna::codon_at(int pos) {
    int value = 0;

    int t_pos;

    for (int i = 0; i < CODON_SIZE; i++) {
        t_pos = pos + i;
        if (t_pos >= seq_.size())
            t_pos -= seq_.size();
        if (seq_[t_pos] == '1')
            value += 1 << (CODON_SIZE - i - 1);
    }

    return value;
}