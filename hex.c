// Copyright (c) 2024 Ole-Christoffer Granmo
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>

#define BOARD_DIM 11 // Define BOARD_DIM with a default value

int neighbors[] = {-(BOARD_DIM + 2) + 1, -(BOARD_DIM + 2), -1, 1, (BOARD_DIM + 2), (BOARD_DIM + 2) - 1};

struct hex_game {
    int board[(BOARD_DIM + 2) * (BOARD_DIM + 2) * 2];
    int open_positions[BOARD_DIM * BOARD_DIM];
    int number_of_open_positions;
    int moves[BOARD_DIM * BOARD_DIM];
    int connected[(BOARD_DIM + 2) * (BOARD_DIM + 2) * 2];
};

void hg_init(struct hex_game *hg) {
    for (int i = 0; i < BOARD_DIM + 2; ++i) {
        for (int j = 0; j < BOARD_DIM + 2; ++j) {
            hg->board[(i * (BOARD_DIM + 2) + j) * 2] = 0;
            hg->board[(i * (BOARD_DIM + 2) + j) * 2 + 1] = 0;

            if (i > 0 && i < BOARD_DIM + 1 && j > 0 && j < BOARD_DIM + 1) {
                hg->open_positions[(i - 1) * BOARD_DIM + j - 1] = i * (BOARD_DIM + 2) + j;
            }

            if (i == 0) {
                hg->connected[(i * (BOARD_DIM + 2) + j) * 2] = 1;
            } else {
                hg->connected[(i * (BOARD_DIM + 2) + j) * 2] = 0;
            }

            if (j == 0) {
                hg->connected[(i * (BOARD_DIM + 2) + j) * 2 + 1] = 1;
            } else {
                hg->connected[(i * (BOARD_DIM + 2) + j) * 2 + 1] = 0;
            }
        }
    }
    hg->number_of_open_positions = BOARD_DIM * BOARD_DIM;
}

int hg_connect(struct hex_game *hg, int player, int position) {
    hg->connected[position * 2 + player] = 1;

    if (player == 0 && position / (BOARD_DIM + 2) == BOARD_DIM) {
        return 1;
    }
    if (player == 1 && position % (BOARD_DIM + 2) == BOARD_DIM) {
        return 1;
    }

    for (int i = 0; i < 6; ++i) {
        int neighbor = position + neighbors[i];
        if (hg->board[neighbor * 2 + player] && !hg->connected[neighbor * 2 + player]) {
            if (hg_connect(hg, player, neighbor)) {
                return 1;
            }
        }
    }
    return 0;
}

int hg_winner(struct hex_game *hg, int player, int position) {
    for (int i = 0; i < 6; ++i) {
        int neighbor = position + neighbors[i];
        if (hg->connected[neighbor * 2 + player]) {
            return hg_connect(hg, player, position);
        }
    }
    return 0;
}

int hg_place_piece_with_strategy(struct hex_game *hg, int player) {
    for (int i = 0; i < hg->number_of_open_positions; ++i) {
        int pos = hg->open_positions[i];
        hg->board[pos * 2 + player] = 1;
        if (hg_winner(hg, player, pos)) {
            return pos;
        }
        hg->board[pos * 2 + player] = 0;
    }

    int opponent = 1 - player;
    for (int i = 0; i < hg->number_of_open_positions; ++i) {
        int pos = hg->open_positions[i];
        hg->board[pos * 2 + opponent] = 1;
        if (hg_winner(hg, opponent, pos)) {
            hg->board[pos * 2 + opponent] = 0;
            hg->board[pos * 2 + player] = 1;
            return pos;
        }
        hg->board[pos * 2 + opponent] = 0;
    }

    int random_empty_position_index = rand() % hg->number_of_open_positions;
    int empty_position = hg->open_positions[random_empty_position_index];
    hg->board[empty_position * 2 + player] = 1;
    hg->open_positions[random_empty_position_index] = hg->open_positions[hg->number_of_open_positions - 1];
    hg->number_of_open_positions--;
    return empty_position;
}

void write_csv_header(FILE *file) {
    for (int i = 1; i <= BOARD_DIM; ++i) {
        for (int j = 1; j <= BOARD_DIM; ++j) {
            fprintf(file, "%d_%d,", i, j);
        }
    }
    fprintf(file, "Winner\n");
}

void save_game_data(struct hex_game *hg, int winner, FILE *file) {
    for (int i = 1; i <= BOARD_DIM; ++i) {
        for (int j = 1; j <= BOARD_DIM; ++j) {
            if (hg->board[(i * (BOARD_DIM + 2) + j) * 2] == 1) {
                fprintf(file, "X,");
            } else if (hg->board[(i * (BOARD_DIM + 2) + j) * 2 + 1] == 2) {
                fprintf(file, "O,");
            } else {
                fprintf(file, ".,");
            }
        }
    }
    fprintf(file, "%d\n", winner);
}

int main() {
    int board_sizes[] = {3, 5, 7, 9, 11, 13};
    int num_sizes = sizeof(board_sizes) / sizeof(board_sizes[0]);

    for (int i = 0; i < num_sizes; ++i) {
        #define BOARD_DIM board_sizes[i]
        printf("Generating data for %dx%d board...\n", BOARD_DIM, BOARD_DIM);

        struct hex_game hg;
        char filename[50];
        sprintf(filename, "hex_game_data_%dx%d_complete.csv", BOARD_DIM, BOARD_DIM);
        FILE *file_complete = fopen(filename, "w");
        write_csv_header(file_complete);

        int target_games = 2000, valid_games = 0;
        while (valid_games < target_games) {
            hg_init(&hg);
            int winner = 0, moves = 0;
            while (!hg_full_board(&hg)) {
                hg_place_piece_with_strategy(&hg, winner % 2);
                moves++;
                if (hg_winner(&hg, winner % 2, moves)) {
                    break;
                }
                winner++;
            }
            save_game_data(&hg, winner, file_complete);
            valid_games++;
        }
        fclose(file_complete);
    }
    return 0;
}
