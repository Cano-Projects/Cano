#ifndef MAIN_H
#define MAIN_H

#define CREATE_UNDO(t, p)                                                      \
    do {                                                                       \
        Undo undo = {0};                                                       \
        undo.type = (t);                                                       \
        undo.start = (p);                                                      \
        state->cur_undo = undo;                                                \
    } while (0)

/* --------------------------- FUNCTIONS --------------------------- */

int main(int argc, char **argv);

#endif // MAIN_H
