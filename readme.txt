mylib.c  ,mydll.c ---------------- co hafuman encrypt decrypt


example code:


    uint32 decodesize = Decrypt(jacq, sizeof jacq, out);
    if (memcmp(out, jacqd, sizeof jacqd ) != 0) {
        while (1);
    }
    Decrypt(guid, sizeof guid, out);
    if (memcmp(out, guidd, sizeof guidd ) != 0) {
        while (1);
    }
    decodesize = Decrypt(dise, sizeof dise, out);
    if (memcmp(out, dised, sizeof dised ) != 0) {
        while (1);
    }