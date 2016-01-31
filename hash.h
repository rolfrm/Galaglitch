
typedef struct _hashstate hashstate;
hashstate * hashstate_new();
void hashstate_reset(hashstate * hs);
void hashstate_update(hashstate * hs, const void * data, size_t len);
u64 hashstate_digest(hashstate * hs);
void hashstate_free(hashstate **hs);
