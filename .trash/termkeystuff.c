
static
int lookup_t_cmp(const void *a, const void *b) {
   lookup_t *la = (lookup_t *) a;
   lookup_t *lb = (lookup_t *) b;
   return (la->sym - lb->sym);
}

static
void finalize_lookup_table(lookup_t *table, int size) {
   qsort((void*) table, size, sizeof(lookup_t), &lookup_t_cmp);
}

