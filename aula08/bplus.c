#include "bplus.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Resultado que sobe para o pai quando uma pagina e dividida.
 * right_offset == 0 significa que nao houve divisao. */
typedef struct bp_split {
    char *key;
    unsigned long right_offset;
} BP_SPLIT;

/* Uma pagina reserva um contador, N+1 ponteiros e N chaves. */
static size_t page_size(unsigned short key_size, size_t capacity)
{
    return sizeof(unsigned long) * (capacity + 2) + key_size * capacity;
}

static bool parameters_valid(unsigned short key_size, unsigned short capacity)
{
    return key_size > 0 && capacity >= 2
        && page_size(key_size, capacity) <= USHRT_MAX;
}

static bool offset_valid(const BP_HEADER *header, unsigned long offset)
{
    return offset >= sizeof(BP_HEADER) && offset <= (unsigned long)LONG_MAX
        && (offset - sizeof(BP_HEADER)) % header->page_size_bytes == 0;
}

static bool header_initialize(BP_HEADER *header, unsigned short key_size,
                              unsigned short capacity)
{
    if (!parameters_valid(key_size, capacity))
        return false;

    memset(header, 0, sizeof(*header));
    memcpy(header->magic, "BP", 2);
    header->tree_height = 1;
    header->left_leaf = sizeof(BP_HEADER);
    header->root_offset = sizeof(BP_HEADER);
    header->key_size = key_size;
    header->page_max_size = capacity;
    header->page_size_bytes = (unsigned short)page_size(key_size, capacity);
    return true;
}

static bool header_valid(const BP_HEADER *header)
{
    return memcmp(header->magic, "BP", 2) == 0
        && parameters_valid(header->key_size, header->page_max_size)
        && header->page_size_bytes
            == page_size(header->key_size, header->page_max_size)
        && header->tree_height > 0
        && offset_valid(header, header->root_offset)
        && offset_valid(header, header->left_leaf);
}

/* capacity pode ser N+1 numa pagina temporaria, nunca gravada em disco. */
static BP_PAGE *page_allocate(unsigned short key_size, size_t capacity)
{
    BP_PAGE *page = malloc(sizeof(*page));
    if (page == NULL)
        return NULL;

    page->buffer = calloc(1, page_size(key_size, capacity));
    if (page->buffer == NULL) {
        free(page);
        return NULL;
    }

    page->count = (unsigned long *)page->buffer;
    page->pointers = (unsigned long *)(page->buffer + sizeof(unsigned long));
    page->keys = page->buffer + sizeof(unsigned long) * (capacity + 2);
    return page;
}

static void page_free(BP_PAGE *page)
{
    if (page != NULL) {
        free(page->buffer);
        free(page);
    }
}

static bool header_write(BP_TREE *tree)
{
    return fseek(tree->f, 0, SEEK_SET) == 0
        && fwrite(&tree->header, sizeof(tree->header), 1, tree->f) == 1;
}

static bool page_write(BP_TREE *tree, unsigned long offset, const BP_PAGE *page)
{
    return offset_valid(&tree->header, offset)
        && fseek(tree->f, (long)offset, SEEK_SET) == 0
        && fwrite(page->buffer, tree->header.page_size_bytes, 1, tree->f) == 1;
}

static BP_PAGE *page_read(BP_TREE *tree, unsigned long offset)
{
    if (!offset_valid(&tree->header, offset))
        return NULL;

    BP_PAGE *page = page_allocate(tree->header.key_size,
                                  tree->header.page_max_size);
    if (page == NULL)
        return NULL;

    if (fseek(tree->f, (long)offset, SEEK_SET) != 0
        || fread(page->buffer, tree->header.page_size_bytes, 1, tree->f) != 1
        || *page->count > tree->header.page_max_size) {
        page_free(page);
        return NULL;
    }
    return page;
}

static bool page_append(BP_TREE *tree, const BP_PAGE *page, unsigned long *offset)
{
    if (fseek(tree->f, 0, SEEK_END) != 0)
        return false;

    long position = ftell(tree->f);
    if (position < 0 || !page_write(tree, (unsigned long)position, page))
        return false;

    *offset = (unsigned long)position;
    return true;
}

/* Tambem serve para liberar uma abertura incompleta, sem gravar nada. */
static void tree_release(BP_TREE *tree)
{
    if (tree == NULL)
        return;
    page_free(tree->root);
    if (tree->f != NULL)
        fclose(tree->f);
    free(tree);
}

bool bp_create_if_not_exists(const char *filename, unsigned short key_size,
                             unsigned short page_max_size)
{
    BP_TREE tree = {0};
    if (filename == NULL
        || !header_initialize(&tree.header, key_size, page_max_size))
        return false;

    tree.root = page_allocate(key_size, page_max_size);
    if (tree.root == NULL)
        return false;

    /* O modificador x impede que um arquivo existente seja sobrescrito. */
    tree.f = fopen(filename, "wbx");
    if (tree.f == NULL) {
        page_free(tree.root);
        return false;
    }

    bool ok = header_write(&tree)
        && page_write(&tree, tree.header.root_offset, tree.root);
    page_free(tree.root);
    if (fclose(tree.f) != 0)
        ok = false;
    if (!ok)
        remove(filename);
    return ok;
}

BP_TREE *bp_open(const char *filename)
{
    if (filename == NULL)
        return NULL;

    BP_TREE *tree = calloc(1, sizeof(*tree));
    if (tree == NULL)
        return NULL;

    tree->f = fopen(filename, "r+b");
    if (tree->f == NULL
        || fread(&tree->header, sizeof(tree->header), 1, tree->f) != 1
        || !header_valid(&tree->header)) {
        tree_release(tree);
        return NULL;
    }

    tree->root = page_read(tree, tree->header.root_offset);
    if (tree->root == NULL
        || (tree->header.tree_height > 1 && *tree->root->count == 0)) {
        tree_release(tree);
        return NULL;
    }
    return tree;
}

void bp_close(BP_TREE *tree)
{
    if (tree != NULL) {
        header_write(tree);
        tree_release(tree);
    }
}

static char *page_key(const BP_TREE *tree, const BP_PAGE *page,
                       unsigned long position)
{
    return page->keys + position * tree->header.key_size;
}

/* Primeira posicao cuja chave e maior ou igual a chave procurada. */
static unsigned long key_position(const BP_TREE *tree, const BP_PAGE *page,
                                   const char *key)
{
    unsigned long position = 0;
    while (position < *page->count
        && memcmp(page_key(tree, page, position), key, tree->header.key_size) < 0)
        ++position;
    return position;
}

/* Um separador e a menor chave do filho a sua direita.
 * Por isso, a igualdade tambem segue para a direita. */
static unsigned long child_position(const BP_TREE *tree, const BP_PAGE *page,
                                     const char *key)
{
    unsigned long position = key_position(tree, page, key);
    if (position < *page->count
        && memcmp(page_key(tree, page, position), key, tree->header.key_size) == 0)
        ++position;
    return position;
}

bool bp_find(BP_TREE *tree, const char key[], unsigned long *ptr)
{
    if (tree == NULL || key == NULL || ptr == NULL)
        return false;

    BP_PAGE *page = tree->root;
    unsigned short height = tree->header.tree_height;
    while (height > 1) {
        unsigned long position = child_position(tree, page, key);
        BP_PAGE *child = page_read(tree, page->pointers[position]);
        if (page != tree->root)
            page_free(page);
        if (child == NULL)
            return false;
        page = child;
        --height;
    }

    unsigned long position = key_position(tree, page, key);
    bool found = position < *page->count
        && memcmp(page_key(tree, page, position), key, tree->header.key_size) == 0;
    if (found)
        *ptr = page->pointers[position];
    if (page != tree->root)
        page_free(page);
    return found;
}

/* Copia uma faixa de chaves para o inicio de outra pagina.
 * Folhas tem um valor por chave; internas tem um filho a mais. */
static void page_copy_part(const BP_TREE *tree, BP_PAGE *destination,
                           const BP_PAGE *source, unsigned long first,
                           unsigned long count, bool leaf)
{
    memcpy(destination->keys, page_key(tree, source, first),
           count * tree->header.key_size);
    memcpy(destination->pointers, source->pointers + first,
           (count + (leaf ? 0 : 1)) * sizeof(unsigned long));
    *destination->count = count;
}

/* Uma unica vaga extra permite inserir antes de escolher onde dividir. */
static BP_PAGE *page_for_split(const BP_TREE *tree, const BP_PAGE *page,
                                bool leaf)
{
    BP_PAGE *temporary = page_allocate(tree->header.key_size,
                                       (size_t)tree->header.page_max_size + 1);
    if (temporary != NULL)
        page_copy_part(tree, temporary, page, 0, *page->count, leaf);
    return temporary;
}

/* memmove permite deslocar faixas sobrepostas dentro da mesma pagina. */
static void page_insert_key(const BP_TREE *tree, BP_PAGE *page,
                             unsigned long position, const char *key)
{
    size_t bytes = (*page->count - position) * tree->header.key_size;
    memmove(page_key(tree, page, position + 1),
            page_key(tree, page, position), bytes);
    memcpy(page_key(tree, page, position), key, tree->header.key_size);
    ++*page->count;
}

static void leaf_insert_entry(const BP_TREE *tree, BP_PAGE *page,
                               unsigned long position, const char *key,
                               unsigned long ptr)
{
    memmove(page->pointers + position + 1, page->pointers + position,
            (*page->count - position) * sizeof(unsigned long));
    page->pointers[position] = ptr;
    page_insert_key(tree, page, position, key);
}

static void internal_insert_entry(const BP_TREE *tree, BP_PAGE *page,
                                   unsigned long position, const BP_SPLIT *split)
{
    /* O filho esquerdo ja existe. Abre-se espaco para o novo filho direito. */
    memmove(page->pointers + position + 2, page->pointers + position + 1,
            (*page->count - position) * sizeof(unsigned long));
    page->pointers[position + 1] = split->right_offset;
    page_insert_key(tree, page, position, split->key);
}

static bool leaf_split(BP_TREE *tree, BP_PAGE *page, unsigned long offset,
                       const BP_PAGE *temporary, BP_SPLIT *split)
{
    unsigned long capacity = tree->header.page_max_size;
    unsigned long left_count = *temporary->count / 2;
    unsigned long right_offset;
    BP_PAGE *right = page_allocate(tree->header.key_size, capacity);
    if (right == NULL)
        return false;

    page_copy_part(tree, right, temporary, left_count,
                   *temporary->count - left_count, true);
    right->pointers[capacity] = page->pointers[capacity];
    bool ok = page_append(tree, right, &right_offset);
    if (ok) {
        page_copy_part(tree, page, temporary, 0, left_count, true);
        page->pointers[capacity] = right_offset;
        ok = page_write(tree, offset, page);
    }
    if (ok) {
        /* A primeira chave da direita e COPIADA para o pai. */
        memcpy(split->key, right->keys, tree->header.key_size);
        split->right_offset = right_offset;
    }
    page_free(right);
    return ok;
}

static bool internal_split(BP_TREE *tree, BP_PAGE *page, unsigned long offset,
                           const BP_PAGE *temporary, BP_SPLIT *split)
{
    unsigned long middle = *temporary->count / 2;
    unsigned long right_offset;
    BP_PAGE *right = page_allocate(tree->header.key_size,
                                   tree->header.page_max_size);
    if (right == NULL)
        return false;

    /* A chave central fica fora das duas paginas: sera PROMOVIDA ao pai. */
    page_copy_part(tree, right, temporary, middle + 1,
                   *temporary->count - middle - 1, false);
    bool ok = page_append(tree, right, &right_offset);
    if (ok) {
        page_copy_part(tree, page, temporary, 0, middle, false);
        ok = page_write(tree, offset, page);
    }
    if (ok) {
        memcpy(split->key, page_key(tree, temporary, middle),
               tree->header.key_size);
        split->right_offset = right_offset;
    }
    page_free(right);
    return ok;
}

static bool insert_leaf(BP_TREE *tree, BP_PAGE *page, unsigned long offset,
                        const char *key, unsigned long ptr, BP_SPLIT *split)
{
    unsigned long position = key_position(tree, page, key);
    if (position < *page->count
        && memcmp(page_key(tree, page, position), key, tree->header.key_size) == 0)
        return false;

    if (*page->count < tree->header.page_max_size) {
        leaf_insert_entry(tree, page, position, key, ptr);
        return page_write(tree, offset, page);
    }

    BP_PAGE *temporary = page_for_split(tree, page, true);
    if (temporary == NULL)
        return false;
    leaf_insert_entry(tree, temporary, position, key, ptr);
    bool ok = leaf_split(tree, page, offset, temporary, split);
    page_free(temporary);
    return ok;
}

static bool insert_internal(BP_TREE *tree, BP_PAGE *page, unsigned long offset,
                            unsigned long position, BP_SPLIT *split)
{
    if (*page->count < tree->header.page_max_size) {
        internal_insert_entry(tree, page, position, split);
        split->right_offset = 0;
        return page_write(tree, offset, page);
    }

    BP_PAGE *temporary = page_for_split(tree, page, false);
    if (temporary == NULL)
        return false;
    internal_insert_entry(tree, temporary, position, split);
    bool ok = internal_split(tree, page, offset, temporary, split);
    page_free(temporary);
    return ok;
}

/* Desce ate a folha; na volta, incorpora eventuais divisoes dos filhos. */
static bool insert_recursive(BP_TREE *tree, BP_PAGE *page, unsigned long offset,
                             unsigned short height, const char *key,
                             unsigned long ptr, BP_SPLIT *split)
{
    if (height == 1)
        return insert_leaf(tree, page, offset, key, ptr, split);

    unsigned long position = child_position(tree, page, key);
    unsigned long child_offset = page->pointers[position];
    BP_PAGE *child = page_read(tree, child_offset);
    if (child == NULL)
        return false;

    bool ok = insert_recursive(tree, child, child_offset, height - 1,
                                key, ptr, split);
    page_free(child);
    if (!ok || split->right_offset == 0)
        return ok;
    return insert_internal(tree, page, offset, position, split);
}

/* Quando a raiz se divide, cria-se um novo nivel com dois filhos. */
static bool grow_root(BP_TREE *tree, const BP_SPLIT *split)
{
    if (tree->header.tree_height == USHRT_MAX)
        return false;

    BP_PAGE *root = page_allocate(tree->header.key_size,
                                  tree->header.page_max_size);
    if (root == NULL)
        return false;

    *root->count = 1;
    memcpy(root->keys, split->key, tree->header.key_size);
    root->pointers[0] = tree->header.root_offset;
    root->pointers[1] = split->right_offset;
    unsigned long offset;
    if (!page_append(tree, root, &offset)) {
        page_free(root);
        return false;
    }

    page_free(tree->root);
    tree->root = root;
    tree->header.root_offset = offset;
    ++tree->header.tree_height;
    return true;
}

bool bp_insert(BP_TREE *tree, const char key[], unsigned long ptr)
{
    if (tree == NULL || key == NULL || tree->header.count == ULONG_MAX)
        return false;

    BP_SPLIT split = {0};
    split.key = malloc(tree->header.key_size);
    if (split.key == NULL)
        return false;

    bool ok = insert_recursive(tree, tree->root, tree->header.root_offset,
                                tree->header.tree_height, key, ptr, &split);
    if (ok && split.right_offset != 0)
        ok = grow_root(tree, &split);
    free(split.key);
    if (!ok)
        return false;

    ++tree->header.count;
    /* Cada insercao concluida grava tambem os metadados da arvore. */
    return header_write(tree) && fflush(tree->f) == 0;
}
